#include "tasking.hpp"

#include "../memory/memory.hpp"

static constexpr uint32_t MAX_TASKS = 8;
static constexpr uint32_t MIN_STACK_SIZE = 1024;
static constexpr uint32_t STACK_ALIGNMENT = 16;

static task g_tasks[MAX_TASKS];
static uint32_t g_task_count = 0;
static uint32_t g_current_task = 0;
static bool g_scheduler_started = false;
static bool g_bootstrap_switch_pending = false;

static inline uint32_t align_up(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

void tasking_init()
{
    g_task_count = 0;
    g_current_task = 0;
    g_scheduler_started = false;
    g_bootstrap_switch_pending = false;

    for (uint32_t i = 0; i < MAX_TASKS; ++i)
    {
        g_tasks[i].id = i;
        g_tasks[i].stack = nullptr;
        g_tasks[i].stack_size = 0;
        g_tasks[i].esp = 0;
        g_tasks[i].active = false;
    }
}

bool task_create(void (*entry)(), uint32_t stack_size)
{
    if (!entry || g_task_count >= MAX_TASKS)
        return false;

    uint32_t aligned_stack_size = align_up(stack_size < MIN_STACK_SIZE ? MIN_STACK_SIZE : stack_size, STACK_ALIGNMENT);
    uint8_t *raw_stack = reinterpret_cast<uint8_t *>(kmalloc(aligned_stack_size));
    if (!raw_stack)
        return false;

    task &t = g_tasks[g_task_count];
    t.id = g_task_count;
    t.stack = reinterpret_cast<uint32_t *>(raw_stack);
    t.stack_size = aligned_stack_size;
    t.active = true;

    uint32_t stack_top = reinterpret_cast<uint32_t>(raw_stack + aligned_stack_size);
    stack_top &= ~(STACK_ALIGNMENT - 1);

    uint32_t *sp = reinterpret_cast<uint32_t *>(stack_top);

    // CPU state expected by iretd.
    *(--sp) = 0x00000202;                        // EFLAGS (IF=1)
    *(--sp) = 0x00000008;                        // CS (kernel code segment)
    *(--sp) = reinterpret_cast<uint32_t>(entry); // EIP

    // pushad layout expected by popad in reverse: EDI, ESI, EBP, ESP(dummy), EBX, EDX, ECX, EAX.
    *(--sp) = 0; // EDI
    *(--sp) = 0; // ESI
    *(--sp) = 0; // EBP
    *(--sp) = 0; // ESP (discarded by popad)
    *(--sp) = 0; // EBX
    *(--sp) = 0; // EDX
    *(--sp) = 0; // ECX
    *(--sp) = 0; // EAX

    t.esp = reinterpret_cast<uint32_t>(sp);
    ++g_task_count;
    return true;
}

void tasking_start()
{
    if (g_task_count == 0)
        return;

    g_scheduler_started = true;
    g_current_task = 0;
    g_bootstrap_switch_pending = true;
}

uint32_t tasking_schedule(uint32_t current_esp)
{
    if (!g_scheduler_started || g_task_count == 0)
        return current_esp;

    if (g_bootstrap_switch_pending)
    {
        g_bootstrap_switch_pending = false;
        return g_tasks[g_current_task].esp;
    }

    g_tasks[g_current_task].esp = current_esp;

    uint32_t next = g_current_task;
    for (uint32_t i = 0; i < g_task_count; ++i)
    {
        next = (next + 1) % g_task_count;
        if (g_tasks[next].active)
        {
            g_current_task = next;
            return g_tasks[next].esp;
        }
    }

    return current_esp;
}
#ifndef TASKING_HPP
#define TASKING_HPP

#include <stdint.h>

struct task
{
    uint32_t id;
    uint32_t *stack;
    uint32_t stack_size;
    uint32_t esp;
    bool active;
};

void tasking_init();
bool task_create(void (*entry)(), uint32_t stack_size);
void tasking_start();
uint32_t tasking_schedule(uint32_t current_esp);

#endif
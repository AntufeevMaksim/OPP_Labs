#include "actual_task.hpp"

#include <stdio.h>


ActualTask::ActualTask(int id) : id_{id} {}

void ActualTask::execute()
{
    printf("Task % d\n", id_);
}

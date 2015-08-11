#pragma once

#include <ctime> //only for the debugging

#include "newcars.h"
#include "storage.h"
#include "toqueues.h"
#include "totravarea.h"
#include "display.h"

void simulation(int modelisationTime, std::map<int, Road>& Roads, std::vector<Car>& Cars, int timestep, std::vector<int> timestepsToPrint, int superior);
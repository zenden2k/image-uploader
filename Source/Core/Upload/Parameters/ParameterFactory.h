
#pragma once

#include <memory>
#include <string>

#include "AbstractParameter.h"
#include "Core/Scripting/Squirrelnc.h"

std::unique_ptr<AbstractParameter> SqTableToParameter(const std::string& name, const std::string& type, Sqrat::Table& table);

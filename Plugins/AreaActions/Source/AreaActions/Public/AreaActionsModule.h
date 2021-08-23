#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAreaActions, All, All);

class FAreaActionsModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;
};
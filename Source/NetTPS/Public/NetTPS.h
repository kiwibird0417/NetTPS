// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define LOCAL_ROLE (UEnum::GetValueAsString<ENetRole>(GetLocalRole()))
#define REMOTE_ROLE (UEnum::GetValueAsString<ENetRole>(GetRemoteRole()))

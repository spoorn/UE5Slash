/**
 * Debug Macros
 */

#pragma once

#define DRAW_SPHERE(Location) if (GetWorld()) DrawDebugSphere(GetWorld(), Location, 25.0f, 24, FColor::Red, false, 30.0f);
#define DRAW_SPHERE_SINGLE_FRAME(Location) if (GetWorld()) DrawDebugSphere(GetWorld(), Location, 25.0f, 24, FColor::Red, false, -1);
#define DRAW_LINE(Start, End) if (GetWorld()) DrawDebugLine(GetWorld(), Start, End, FColor::Green, true, -1.0f, 0, 1.0);
#define DRAW_LINE_SINGLE_FRAME(Start, End) if (GetWorld()) DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, -1.0f, 0);
#define DRAW_POINT(Location) if (GetWorld()) DrawDebugPoint(GetWorld(), Location, 15.0f, FColor::Green, true);
#define DRAW_POINT_SINGLE_FRAME(Location) if (GetWorld()) DrawDebugPoint(GetWorld(), Location, 15.0f, FColor::Green, false, -1);
#define DRAW_VECTOR(Start, End) DRAW_LINE(Start, End); \
		DRAW_POINT(End);
#define DRAW_VECTOR_SINGLE_FRAME(Start, End) DRAW_LINE_SINGLE_FRAME(Start, End); \
		DRAW_POINT_SINGLE_FRAME(End);
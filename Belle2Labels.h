//
//   @file    Belle2Labels.h


#ifndef __BELLE2LABELS_H
#define __BELLE2LABELS_H

#include "Rtypes.h"

void BELLE2Label(Double_t x, Double_t y, const char* text = NULL, Color_t color = 1);

void BELLE2LabelOld(Double_t x, Double_t y, bool Preliminary = false, Color_t color = 1);

void BELLE2Version(const char* version = NULL, Double_t x = 0.88, Double_t y = 0.975, Color_t color = 1);

#endif // __BELLE2LABELS_H

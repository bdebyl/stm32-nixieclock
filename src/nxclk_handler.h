/* Copyright 2021 Bastian de Byl */
#ifndef SRC_NXCLK_HANDLER_H_
#define SRC_NXCLK_HANDLER_H_

#include "nxclk_common.h"

void nxclk_handle(nxclk_mode mode);
void nxclk_set_mode(nxclk_mode mode);
nxclk_mode nxclk_get_mode(void);
void nxclk_handler_start(void);

#endif  // SRC_NXCLK_HANDLER_H_


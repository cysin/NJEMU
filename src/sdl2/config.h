/******************************************************************************

	config.h

	SDL2 configuration file management

******************************************************************************/

#ifndef SDL2_CONFIG_H
#define SDL2_CONFIG_H

void load_settings(void);
void save_settings(void);
void load_gamecfg(const char *name);
void save_gamecfg(const char *name);

#endif /* SDL2_CONFIG_H */

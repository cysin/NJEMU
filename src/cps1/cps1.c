/******************************************************************************

	cps1.c

	CPS1���ߥ��`����󥳥�

******************************************************************************/

#include "cps1.h"


/******************************************************************************
	���`�����v��
******************************************************************************/

/*--------------------------------------------------------
	CPS1���ߥ��`�������ڻ�
--------------------------------------------------------*/

static int cps1_init(void)
{
	printf("[DEBUG] cps1_init: Starting cps1_driver_init()\n");
	fflush(stdout);
	if (!cps1_driver_init())
	{
		printf("[DEBUG] cps1_init: cps1_driver_init() FAILED\n");
		fflush(stdout);
		return 0;
	}

	printf("[DEBUG] cps1_init: cps1_driver_init() OK\n");
	fflush(stdout);

	msg_printf(TEXT(DONE2));
	msg_screen_clear();

	video_clear_screen();

#ifdef ADHOC
	printf("[DEBUG] cps1_init: Starting cps1_video_init()\n");
	fflush(stdout);
	if (!cps1_video_init())
	{
		printf("[DEBUG] cps1_init: cps1_video_init() FAILED\n");
		fflush(stdout);
		return 0;
	}

	if (adhoc_enable)
	{
		sprintf(adhoc_matching, "%s_%s", PBPNAME_STR, game_name);

		Loop = LOOP_EXEC;//

		if (adhocInit(adhoc_matching) == 0)
		{
			if ((adhoc_server = adhocSelect()) >= 0)
			{
				video_clear_screen();

				if (adhoc_server)
				{
					option_controller = INPUT_PLAYER1;

					return adhoc_send_state(NULL);
				}
				else
				{
					option_controller = INPUT_PLAYER2;

					return adhoc_recv_state(NULL);
				}
			}
		}

		Loop = LOOP_BROWSER;
		return 0;
	}

	printf("[DEBUG] cps1_init: Completed successfully\n");
	fflush(stdout);
	return 1;
#else
	printf("[DEBUG] cps1_init: Starting cps1_video_init()\n");
	fflush(stdout);
	int result = cps1_video_init();
	printf("[DEBUG] cps1_init: cps1_video_init() returned %d\n", result);
	fflush(stdout);
	return result;
#endif
}


/*--------------------------------------------------------
	CPS1���ߥ��`�����ꥻ�å�
--------------------------------------------------------*/

static void cps1_reset(void)
{
	video_set_mode(16);
	video_clear_screen();

	Loop = LOOP_EXEC;

	autoframeskip_reset();

	cps1_driver_reset();
	cps1_video_reset();

	timer_reset();
	input_reset();
	sound_reset();

	blit_clear_all_sprite();
}

/*--------------------------------------------------------
	CPS���ߥ��`�����K��
--------------------------------------------------------*/

static void cps1_exit(void)
{
	video_set_mode(32);
	video_clear_screen();

	ui_popup_reset();

	video_clear_screen();
	msg_screen_init(WP_LOGO, ICON_SYSTEM, TEXT(EXIT_EMULATION2));

	msg_printf(TEXT(PLEASE_WAIT2));

	cps1_video_exit();
	cps1_driver_exit();

#ifdef ADHOC
	if (!adhoc_enable)
#endif
	{
#ifdef COMMAND_LIST
		free_commandlist();
#endif
		save_gamecfg(game_name);
	}

	msg_printf(TEXT(DONE2));

#ifdef ADHOC
	if (adhoc_enable) adhocTerm();
#endif

	show_exit_screen();
}

/*--------------------------------------------------------
	cheats
--------------------------------------------------------*/

extern int cheat_num;
extern gamecheat_t* gamecheat[];

static void apply_cheat()
{
	gamecheat_t *a_cheat = NULL;
	cheat_option_t *a_cheat_option = NULL;
	cheat_value_t *a_cheat_value = NULL;
	int c,j;

   for( c = 0; c < cheat_num; c++)
   { //arreglo de cheats
	a_cheat = gamecheat[c];
    if( a_cheat == NULL)
		break; //seguro

    if( a_cheat->curr_option == 0)//se asume que el option 0 es el disable
		continue;

    //Se busca cual es el option habilitado
    a_cheat_option = a_cheat->cheat_option[ a_cheat->curr_option];
    if( a_cheat_option == NULL)
		break; //seguro

		//Se ejecutan todos los value del cheat option
		for(  j = 0; j< a_cheat_option->num_cheat_values; j++)
		{
		a_cheat_value = a_cheat_option->cheat_value[j];
			if( a_cheat_value == NULL)
				break;//seguro
				m68000_write_memory_8(a_cheat_value->address,  a_cheat_value->value);

		}
    }
}

/*--------------------------------------------------------
	CPS1���ߥ��`�����g��
--------------------------------------------------------*/

static void cps1_run(void)
{
	printf("[DEBUG] cps1_run: Entered main loop\n");
	fflush(stdout);

	while (Loop >= LOOP_RESET)
	{
		printf("[DEBUG] cps1_run: Calling cps1_reset()\n");
		fflush(stdout);
		cps1_reset();
		printf("[DEBUG] cps1_run: cps1_reset() complete, entering LOOP_EXEC\n");
		printf("[DEBUG] cps1_run: Sleep=%d, Loop=%d\n", Sleep, Loop);
		fflush(stdout);

		int frame_count = 0;
		while (Loop == LOOP_EXEC)
		{
			if (frame_count == 0)
			{
				printf("[DEBUG] cps1_run: First iteration - Sleep=%d, Loop=%d\n", Sleep, Loop);
				fflush(stdout);
			}

			if (Sleep)
			{
				printf("[DEBUG] cps1_run: Sleep is set, entering sleep loop\n");
				fflush(stdout);
				do
				{
					sceKernelDelayThread(5000000);
				} while (Sleep);

				autoframeskip_reset();
			}

			if (frame_count == 0)
			{
				printf("[DEBUG] cps1_run: Calling apply_cheat()\n");
				fflush(stdout);
			}
			apply_cheat(); //davex cheat

			if (frame_count == 0)
			{
				printf("[DEBUG] cps1_run: Calling timer_update_cpu()\n");
				fflush(stdout);
			}
			timer_update_cpu();

			if (frame_count == 0)
			{
				printf("[DEBUG] cps1_run: Calling update_screen()\n");
				fflush(stdout);
			}
			update_screen();

			if (frame_count == 0)
			{
				printf("[DEBUG] cps1_run: Calling update_inputport()\n");
				fflush(stdout);
			}
			update_inputport();

			frame_count++;
			if (frame_count == 1 || frame_count == 60 || (frame_count % 300 == 0))
			{
				printf("[DEBUG] cps1_run: Frame %d\n", frame_count);
				fflush(stdout);
			}
		}

		printf("[DEBUG] cps1_run: Exited LOOP_EXEC, Loop=%d\n", Loop);
		fflush(stdout);
		video_clear_screen();
		sound_mute(1);
	}

	printf("[DEBUG] cps1_run: Exited main loop\n");
	fflush(stdout);
}


/******************************************************************************
	�����`�Х��v��
******************************************************************************/

/*--------------------------------------------------------
	CPS1���ߥ��`�����ᥤ��
--------------------------------------------------------*/

void cps1_main(void)
{
	Loop = LOOP_RESET;

	while (Loop >= LOOP_RESTART)
	{
		Loop = LOOP_EXEC;

		ui_popup_reset();

		fatal_error = 0;

		video_clear_screen();

		printf("[DEBUG] Starting memory_init()\n");
		fflush(stdout);
		if (memory_init())
		{
			printf("[DEBUG] memory_init() OK, starting sound_init()\n");
			fflush(stdout);
			if (sound_init())
			{
				printf("[DEBUG] sound_init() OK, starting input_init()\n");
				fflush(stdout);
				if (input_init())
				{
					printf("[DEBUG] input_init() OK, starting cps1_init()\n");
					fflush(stdout);
					if (cps1_init())
					{
						printf("[DEBUG] cps1_init() OK, starting cps1_run()\n");
						fflush(stdout);
						cps1_run();
						printf("[DEBUG] cps1_run() exited\n");
						fflush(stdout);
					}
					else
					{
						printf("[DEBUG] cps1_init() FAILED\n");
						fflush(stdout);
					}
					cps1_exit();
				}
				input_shutdown();
			}
			sound_exit();
		}
		memory_shutdown();
		show_fatal_error();
	}
}

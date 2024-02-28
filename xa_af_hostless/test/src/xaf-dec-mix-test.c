/*
* Copyright (c) 2015-2024 Cadence Design Systems Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef PACK_WS_DUMMY
#include "audio/xa_mp3_dec_api.h"
#endif //PACK_WS_DUMMY
#include "audio/xa-mimo-mix-api.h"
#include "xaf-utils-test.h"
#include "xaf-fio-test.h"

#define PRINT_USAGE FIO_PRINTF(stdout, "\nUsage: %s -infile:filename.mp3 ... -outfile:filename.pcm\n\n", argv[0]); \
                    FIO_PRINTF(stdout, "    Up to 4 input files are supported in the commandline.\n\n"); \
                    FIO_PRINTF(stdout, "NOTE: Input stream params (sample rate, bits-per-sample, channels) should match with that of the Mixer config params.\n\n");\
                    mimo_mix_component_footnote();


#define AUDIO_FRMWK_BUF_SIZE   (256 << 8)
#define AUDIO_COMP_BUF_SIZE    (1024 << 7) + (1024 << 5) + (1024 << 3) + (1024 << 5) + (1024 << 4)

#define NUM_COMP_IN_GRAPH       3
#define MAX_INP_STRMS           4

//component parameters
#define MP3_DEC_PCM_WIDTH       16

#define MIMO_MIX_SAMPLE_RATE         44100
#define MIMO_MIX_IN_NUM_CH           2
#define MIMO_MIX_PCM_WIDTH           16

unsigned int num_bytes_read, num_bytes_write;
extern int audio_frmwk_buf_size;
extern int audio_comp_buf_size;
double strm_duration;

#ifdef XAF_PROFILE
    extern long long tot_cycles, frmwk_cycles, fread_cycles, fwrite_cycles;
    extern long long dsp_comps_cycles, dec_cycles, mimo_mix_cycles;
    extern double dsp_mcps;
#endif

/* Dummy unused functions */
XA_ERRORCODE xa_mixer(xa_codec_handle_t var1, WORD32 var2, WORD32 var3, pVOID var4){return 0;}
XA_ERRORCODE xa_aac_decoder(xa_codec_handle_t var1, WORD32 var2, WORD32 var3, pVOID var4){return 0;}
XA_ERRORCODE xa_pcm_gain(xa_codec_handle_t var1, WORD32 var2, WORD32 var3, pVOID var4){return 0;}
XA_ERRORCODE xa_mp3_encoder(xa_codec_handle_t var1, WORD32 var2, WORD32 var3, pVOID var4){return 0;}
XA_ERRORCODE xa_src_pp_fx(xa_codec_handle_t var1, WORD32 var2, WORD32 var3, pVOID var4){return 0;}
XA_ERRORCODE xa_renderer(xa_codec_handle_t var1, WORD32 var2, WORD32 var3, pVOID var4){return 0;}
XA_ERRORCODE xa_capturer(xa_codec_handle_t var1, WORD32 var2, WORD32 var3, pVOID var4){return 0;}
XA_ERRORCODE xa_amr_wb_decoder(xa_codec_handle_t var1, WORD32 var2, WORD32 var3, pVOID var4){return 0;}
XA_ERRORCODE xa_hotword_decoder(xa_codec_handle_t p_xa_module_obj, WORD32 i_cmd, WORD32 i_idx, pVOID pv_value){return 0;}
XA_ERRORCODE xa_vorbis_decoder(xa_codec_handle_t var1, WORD32 var2, WORD32 var3, pVOID var4){return 0;}
XA_ERRORCODE xa_dummy_aec22(xa_codec_handle_t p_xa_module_obj, WORD32 i_cmd, WORD32 i_idx, pVOID pv_value) {return 0;}
XA_ERRORCODE xa_dummy_aec23(xa_codec_handle_t p_xa_module_obj, WORD32 i_cmd, WORD32 i_idx, pVOID pv_value) {return 0;}
XA_ERRORCODE xa_pcm_split(xa_codec_handle_t p_xa_module_obj, WORD32 i_cmd, WORD32 i_idx, pVOID pv_value) {return 0;}
XA_ERRORCODE xa_mimo_mix(xa_codec_handle_t p_xa_module_obj, WORD32 i_cmd, WORD32 i_idx, pVOID pv_value) {return 0;}
XA_ERRORCODE xa_dummy_wwd(xa_codec_handle_t p_xa_module_obj, WORD32 i_cmd, WORD32 i_idx, pVOID pv_value) {return 0;}
XA_ERRORCODE xa_dummy_hbuf(xa_codec_handle_t p_xa_module_obj, WORD32 i_cmd, WORD32 i_idx, pVOID pv_value) {return 0;}
XA_ERRORCODE xa_opus_encoder(xa_codec_handle_t p_xa_module_obj, WORD32 i_cmd, WORD32 i_idx, pVOID pv_value) {return 0;}
XA_ERRORCODE xa_dummy_wwd_msg(xa_codec_handle_t var1, WORD32 var2, WORD32 var3, pVOID var4){return 0;}
XA_ERRORCODE xa_dummy_hbuf_msg(xa_codec_handle_t var1, WORD32 var2, WORD32 var3, pVOID var4){return 0;}
XA_ERRORCODE xa_opus_decoder(xa_codec_handle_t p_xa_module_obj, WORD32 i_cmd, WORD32 i_idx, pVOID pv_value) {return 0;}
XA_ERRORCODE xa_microspeech_fe(xa_codec_handle_t var1, WORD32 var2, WORD32 var3, pVOID var4){return 0;}
XA_ERRORCODE xa_microspeech_inference(xa_codec_handle_t var1, WORD32 var2, WORD32 var3, pVOID var4){return 0;}
XA_ERRORCODE xa_person_detect_inference(xa_codec_handle_t var1, WORD32 var2, WORD32 var3, pVOID var4){return 0;}
XA_ERRORCODE xa_keyword_detection_inference(xa_codec_handle_t var1, WORD32 var2, WORD32 var3, pVOID var4){return 0;}

static int mp3_setup(void *p_decoder)
{
#ifndef PACK_WS_DUMMY
    int param[2];

    param[0] = XA_MP3DEC_CONFIG_PARAM_PCM_WDSZ;
    param[1] = MP3_DEC_PCM_WIDTH;

    return(xaf_comp_set_config(p_decoder, 1, &param[0]));
#else //PACK_WS_DUMMY
    return 0;
#endif //PACK_WS_DUMMY
}

static int mimo_mix_setup(void *p_mimo_mix, xaf_format_t *p_format)
{
    int param[6];

    param[0] = XA_MIMO_MIX_CONFIG_PARAM_SAMPLE_RATE;
    param[1] = p_format->sample_rate;
    param[2] = XA_MIMO_MIX_CONFIG_PARAM_CHANNELS;
    param[3] = p_format->channels;
    param[4] = XA_MIMO_MIX_CONFIG_PARAM_PCM_WIDTH;
    param[5] = p_format->pcm_width;

    return(xaf_comp_set_config(p_mimo_mix, 3, &param[0]));
}

static int get_comp_config(void *p_comp, xaf_format_t *comp_format)
{
#ifndef PACK_WS_DUMMY
    int param[6];
    int ret;

    TST_CHK_PTR(p_comp, "get_comp_config");
    TST_CHK_PTR(comp_format, "get_comp_config");

    param[0] = XA_MP3DEC_CONFIG_PARAM_NUM_CHANNELS;
    param[2] = XA_MP3DEC_CONFIG_PARAM_PCM_WDSZ;
    param[4] = XA_MP3DEC_CONFIG_PARAM_SAMP_FREQ;


    ret = xaf_comp_get_config(p_comp, 3, &param[0]);
    if(ret < 0)
        return ret;

    comp_format->channels = param[1];
    comp_format->pcm_width = param[3];
    comp_format->sample_rate = param[5];
#endif //PACK_WS_DUMMY

    return 0;
}

static int check_format(void *p_comp, xaf_format_t *p_format, int *format_match)
{
    xaf_format_t dec_format;
    int ret;

    TST_CHK_PTR(p_comp, "check_format");
    TST_CHK_PTR(p_format, "check_format");
    TST_CHK_PTR(format_match, "check_format");

    *format_match = 0;

    ret = get_comp_config(p_comp, &dec_format);
    if(ret < 0)
        return ret;

    if ((dec_format.channels == p_format->channels) &&
        (dec_format.pcm_width == p_format->pcm_width) &&
        (dec_format.sample_rate == p_format->sample_rate))
        {
            *format_match = 1;
        }
        else
        {
            fprintf(stderr, "\nStream parameter mismatch with mimo_mix!!\n\n");
            return -1;
        }
    return 0;
}

void fio_quit()
{
    return;
}

int main_task(int argc, char **argv)
{
    void *p_adev = NULL;
    void* p_decoder[MAX_INP_STRMS] = {0};
    void * p_mimo_mix = NULL;

    xaf_comp_status dec_status;
    xaf_format_t mimo_mix_format;
    int dec_info[4];

    void *p_input[MAX_INP_STRMS], *p_output;

    xf_thread_t dec_thread[MAX_INP_STRMS];

    unsigned char dec_stack[MAX_INP_STRMS][STACK_SIZE];
    xf_id_t dec_id[MAX_INP_STRMS];
    xf_id_t comp_id_mimo_mix = "mimo-proc41/mimo_mix4";
    int (*dec_setup[MAX_INP_STRMS])(void *p_comp);

    xf_thread_t mimo_mix_thread;
    unsigned char mimo_mix_stack[STACK_SIZE];

    void *dec_thread_args[MAX_INP_STRMS][NUM_THREAD_ARGS];
    void *mimo_mix_thread_args[NUM_THREAD_ARGS];

    const char *ext;
    FILE *fp, *ofp;
    void *dec_inbuf[MAX_INP_STRMS][2];
    int buf_length = XAF_INBUF_SIZE;
    int read_length;
    int input_over = 0;
    int i, j;
    int num_strms = 0;
    int format_match = 0;

    pUWORD8 ver_info[3] = {0,0,0};    //{ver,lib_rev,api_rev}
    unsigned short board_id = 0;
    xaf_comp_type comp_type;
    mem_obj_t* mem_handle;
    int num_comp = NUM_COMP_IN_GRAPH;

#ifdef XAF_PROFILE
    frmwk_cycles = 0;
    fread_cycles = 0;
    fwrite_cycles = 0;
    dsp_comps_cycles = 0;
    dec_cycles = 0;
    mimo_mix_cycles = 0;
    tot_cycles = 0;
    num_bytes_read = 0;
    num_bytes_write = 0;
#endif


    memset(&mimo_mix_format, 0, sizeof(xaf_format_t));
    audio_frmwk_buf_size = AUDIO_FRMWK_BUF_SIZE;
    audio_comp_buf_size = AUDIO_COMP_BUF_SIZE;

    // NOTE: set_wbna() should be called before any other dynamic
    // adjustment of the region attributes for cache.
    set_wbna(&argc, argv);

    /* ...start xos */
    board_id = start_rtos();

    /* ...get xaf version info*/
    TST_CHK_API(xaf_get_verinfo(ver_info), "xaf_get_verinfo");

    /* ...show xaf version info*/
    TST_CHK_API(print_verinfo(ver_info,(pUWORD8)"\'2 Mp3 Decoder + Mixer\'"), "print_verinfo");

    /* ...initialize tracing facility */
    TRACE_INIT("Xtensa Audio Framework - /'Mixer/' Sample App");

    mimo_mix_format.sample_rate = MIMO_MIX_SAMPLE_RATE;
    mimo_mix_format.channels    = MIMO_MIX_IN_NUM_CH;
    mimo_mix_format.pcm_width   = MIMO_MIX_PCM_WIDTH;

    /* ...check input arguments */
    if (argc < 3 || argc > (MAX_INP_STRMS+2))
    {
        PRINT_USAGE;
        return -1;
    }

    argc--;
    for (i=0; i<argc; i++)
    {
        char *filename_ptr;
        // Open input files
        if(NULL != strstr(argv[i+1], "-infile:"))
        {
            filename_ptr = (char *)&(argv[i+1][8]);
            ext = strrchr(argv[i+1], '.');
			if(ext!=NULL)
			{
				ext++;
				if (!strcmp(ext, "mp3")) {
					dec_id[i]    = "audio-decoder/mp3";
					dec_setup[i] = mp3_setup;
				}
				else {
					FIO_PRINTF(stderr, "Unknown Decoder Extension '%s'\n", ext);
                    return -1;
				}
			}
			else
			{
				FIO_PRINTF(stderr, "Failed to open infile %d\n",i+1);
                return -1;
			}
            /* ...open file */
            if ((fp = fio_fopen(filename_ptr, "rb")) == NULL)
            {
               FIO_PRINTF(stderr, "Failed to open '%s': %d\n", argv[i+1], errno);
               return -1;
            }
            p_input[i] = fp;
            num_strms++;
        }
        // Open output file
        else if(NULL != strstr(argv[i+1], "-outfile:"))
        {
            filename_ptr = (char *)&(argv[i+1][9]);

            if ((ofp = fio_fopen(filename_ptr, "wb")) == NULL)
            {
               FIO_PRINTF(stderr, "Failed to open '%s': %d\n", filename_ptr, errno);
               return -1;
            }
        }
        else
        {
            PRINT_USAGE;
            return -1;
        }
    }

    p_output = ofp;

    if(num_strms == 0 || ofp == NULL)
    {
        PRINT_USAGE;
        return -1;
    }

    mem_handle = mem_init();


    xaf_adev_config_t adev_config;
    TST_CHK_API(xaf_adev_config_default_init(&adev_config), "xaf_adev_config_default_init");

    adev_config.mem_pool[XAF_MEM_ID_DEV].size =  audio_frmwk_buf_size;
    adev_config.mem_pool[XAF_MEM_ID_COMP].size = audio_comp_buf_size;
    adev_config.core = XF_CORE_ID;

    TST_CHK_API_ADEV_OPEN(p_adev, adev_config,  "xaf_adev_open");

    FIO_PRINTF(stdout,"Audio Device Ready\n");

    TST_CHK_API(xaf_comp_config_default_init(&comp_config), "xaf_comp_config_default_init");
    comp_config.mem_pool_type[XAF_MEM_POOL_TYPE_COMP_INPUT] = XAF_MEM_ID_COMP_FAST;
    comp_config.mem_pool_type[XAF_MEM_POOL_TYPE_COMP_APP_OUTPUT] = XAF_MEM_ID_DEV_FAST;
    TST_CHK_API_COMP_CREATE_USER_CFG_CHANGE(p_adev, XF_CORE_ID, &p_mimo_mix, "mimo-proc41/mimo_mix4", 0, 1, NULL, XAF_MIMO_PROC_41, "xaf_comp_create");
    TST_CHK_API(mimo_mix_setup(p_mimo_mix, &mimo_mix_format), "mimo_mix_setup");

    for (i=0; i<num_strms; i++)
    {
        /* ...create decoder component */
        TST_CHK_API_COMP_CREATE(p_adev, XF_CORE_ID, &p_decoder[i], dec_id[i], 2, 0, &dec_inbuf[i][0], XAF_DECODER, "xaf_comp_create");
        TST_CHK_API((dec_setup[i])(p_decoder[i]), "dec_setup");


        TST_CHK_API(xaf_comp_process(p_adev, p_decoder[i], NULL, 0, XAF_START_FLAG), "xaf_comp_process");
        for (j=0; j<2; j++)
        {
            TST_CHK_API(read_input(dec_inbuf[i][j], buf_length, &read_length, p_input[i], XAF_DECODER), "read_input");

            if (read_length)
                TST_CHK_API(xaf_comp_process(p_adev, p_decoder[i], dec_inbuf[i][j], read_length, XAF_INPUT_READY_FLAG), "xaf_comp_process");
            else
            {
                TST_CHK_API(xaf_comp_process(p_adev, p_decoder[i], NULL, 0, XAF_INPUT_OVER_FLAG), "xaf_comp_process");
                break;
            }
        }

        while (1)
        {
            TST_CHK_API(xaf_comp_get_status(p_adev, p_decoder[i], &dec_status, &dec_info[0]), "xaf_comp_get_status");

            if (dec_status == XAF_INIT_DONE || dec_status == XAF_EXEC_DONE) break;

            if (dec_status == XAF_NEED_INPUT && !input_over)
            {
                void *p_buf = (void *) dec_info[0];
                int size    = dec_info[1];

                TST_CHK_API(read_input(p_buf, size, &read_length, p_input[i], XAF_DECODER), "read_input");

                if (read_length)
                    TST_CHK_API(xaf_comp_process(p_adev, p_decoder[i], p_buf, read_length, XAF_INPUT_READY_FLAG), "xaf_comp_process");
                else
                {
                    TST_CHK_API(xaf_comp_process(p_adev, p_decoder[i], NULL, 0, XAF_INPUT_OVER_FLAG), "xaf_comp_process");
                    break;
                }
            }
        }

        if (dec_status != XAF_INIT_DONE)
        {
            FIO_PRINTF(stderr, "Failed to init");
            TST_CHK_API(ADEV_CLOSE_SIGNAL, "Stream Initialization");
        }

        TST_CHK_API(check_format(p_decoder[i], &mimo_mix_format, &format_match), "check_format");

        if (format_match)
        {
            TST_CHK_API(xaf_connect(p_decoder[i], 1, p_mimo_mix, i, 4), "xaf_connect");
        }
        else
        {
            FIO_PRINTF(stderr, "Failed to connect");
            TST_CHK_API(ADEV_CLOSE_SIGNAL, "xaf_connect");
        }
    }

    TST_CHK_API(xaf_comp_process(p_adev, p_mimo_mix, NULL, 0, XAF_START_FLAG), "xaf_comp_process");
    TST_CHK_API(xaf_comp_get_status(p_adev, p_mimo_mix, &dec_status, &dec_info[0]), "xaf_comp_get_status");

    if (dec_status != XAF_INIT_DONE)
    {
        FIO_PRINTF(stderr, "Failed to init");
        TST_CHK_API(ADEV_CLOSE_SIGNAL, "Stream Initialization");
    }

#ifdef XAF_PROFILE
    clk_start();

#endif

    comp_type = XAF_DECODER;
    for (i=0; i<num_strms; i++)
    {
        dec_thread_args[i][0] = p_adev;
        dec_thread_args[i][1] = p_decoder[i];
        dec_thread_args[i][2] = p_input[i];
        dec_thread_args[i][3] = p_output;
        dec_thread_args[i][4] = &comp_type;
        dec_thread_args[i][5] = (void *)dec_id[i];
        dec_thread_args[i][6] = (void *)&i;
        __xf_thread_create(&dec_thread[i], comp_process_entry, dec_thread_args[i], "Decoder Thread", dec_stack[i], STACK_SIZE, XAF_APP_THREADS_PRIORITY);
    }

    comp_type = XAF_MIMO_PROC_41;
    mimo_mix_thread_args[0] = p_adev;
    mimo_mix_thread_args[1] = p_mimo_mix;
    mimo_mix_thread_args[2] = NULL;
    mimo_mix_thread_args[3] = p_output;
    mimo_mix_thread_args[4] = &comp_type;
    mimo_mix_thread_args[5] = (void *)comp_id_mimo_mix;
    mimo_mix_thread_args[6] = (void *)&i;
    __xf_thread_create(&mimo_mix_thread, comp_process_entry, &mimo_mix_thread_args[0], "Mixer Thread", mimo_mix_stack, STACK_SIZE, XAF_APP_THREADS_PRIORITY);

    for (i=0; i<num_strms; i++)
    {
        __xf_thread_join(&dec_thread[i], NULL);
    }
    __xf_thread_join(&mimo_mix_thread, NULL);

#ifdef XAF_PROFILE
    compute_total_frmwrk_cycles();
    clk_stop();

#endif

    {
        /* collect memory stats before closing the device */
        WORD32 meminfo[3 + XAF_MEM_ID_MAX], k;
        if(xaf_get_mem_stats(p_adev, adev_config.core, &meminfo[0]))
        {
            FIO_PRINTF(stdout,"Init is incomplete, reliable memory stats are unavailable.\n");
        }
        else
        {
            FIO_PRINTF(stderr,"Local Memory used by DSP Components, in bytes            : %8d of %8d\n", meminfo[0], adev_config.mem_pool[XAF_MEM_ID_COMP].size);
            FIO_PRINTF(stderr,"Shared Memory used by Components and Framework, in bytes : %8d of %8d\n", meminfo[1], adev_config.mem_pool[XAF_MEM_ID_DEV].size);
            FIO_PRINTF(stderr,"Local Memory used by Framework, in bytes                 : %8d\n", meminfo[2]);

            for(k = XAF_MEM_ID_COMP+1, i=5 ; k <= XAF_MEM_ID_COMP_MAX ; k++, i++)
            {
                if(meminfo[i])
                {
                    FIO_PRINTF(stderr,"Local Memory type[%d] used by DSP Components, in bytes    : %8d of %8d\n", k, meminfo[i], adev_config.mem_pool[k].size);
                }
            }
            for(k = XAF_MEM_ID_DEV+1 ; k<=XAF_MEM_ID_DEV_MAX ; k++, i++)
            {
                if(meminfo[i])
                {
                    FIO_PRINTF(stderr,"Shared Memory used by Components and Framework, in bytes (type[%d]): %8d of %8d\n", k, meminfo[i], adev_config.mem_pool[k].size);
                }
            }
        }
    }
    /* ...exec done, clean-up */
    for (i=0; i<num_strms; i++)
    {
        __xf_thread_destroy(&dec_thread[i]);
        TST_CHK_API(xaf_comp_delete(p_decoder[i]), "xaf_comp_delete");
        if (p_input[i]) fio_fclose(p_input[i]);
    }

    __xf_thread_destroy(&mimo_mix_thread);
    TST_CHK_API(xaf_comp_delete(p_mimo_mix), "xaf_comp_delete");
    TST_CHK_API_ADEV_CLOSE(p_adev, XAF_ADEV_NORMAL_CLOSE, adev_config, "xaf_adev_close");
    FIO_PRINTF(stdout,"Audio device closed\n\n");

    mem_exit();

    dsp_comps_cycles = dec_cycles + mimo_mix_cycles;

    dsp_mcps = compute_comp_mcps(num_bytes_write, dsp_comps_cycles, mimo_mix_format, &strm_duration);

    TST_CHK_API(print_mem_mcps_info(mem_handle, num_comp), "print_mem_mcps_info");

    if (p_output) fio_fclose(p_output);

    fio_quit();

    /* ...deinitialize tracing facility */
    TRACE_DEINIT();

    return 0;
}


//
// begin license header
//
// This file is part of Pixy CMUcam5 or "Pixy" for short
//
// All Pixy source code is provided under the terms of the
// GNU General Public License v2 (http://www.gnu.org/licenses/gpl-2.0.html).
// Those wishing to use Pixy source code, software and/or
// technologies under different licensing terms should contact us at
// cmucam@cs.cmu.edu. Such licensing terms are available for
// all portions of the Pixy codebase presented here.
//
// end license header
//

#include <stdio.h>
#include "progvideo.h"
#include "pixy_init.h"
#include "camera.h"
#include "blobs.h"
#include "conncomp.h"
#include "param.h"
#include "led.h"
#include "smlink.hpp"
#include "misc.h"
#include "pixyvals.h"
#include "serial.h"
#include "calc.h"
#include <string.h>

extern "C"{
#include <zbar.h>
}

static uint8_t g_rgbSize = VIDEO_RGB_SIZE;
static zbar_image_scanner_t *scanner = NULL;

REGISTER_PROG(ProgVideo, PROG_NAME_VIDEO, "continuous stream of raw camera frames", PROG_VIDEO_MIN_TYPE, PROG_VIDEO_MAX_TYPE);
ProgVideo::ProgVideo(uint8_t progIndex)
{	
	if (g_execArg==0)
		cam_setMode(CAM_MODE0);
	else
		cam_setMode(CAM_MODE1);

	// run m0 
	exec_runM0(1);
	SM_OBJECT->currentLine = 0;
	SM_OBJECT->stream = 1;
}

ProgVideo::~ProgVideo()
{
	exec_stopM0();
}

int ProgVideo::loop(char *status)
{
	while(SM_OBJECT->currentLine < CAM_RES2_HEIGHT-2)
	{
		if (SM_OBJECT->stream==0)
			printf("not streaming\n");
	}
	SM_OBJECT->stream = 0; // pause after frame grab is finished
	
	// send over USB 
	if (g_execArg==0)
		cam_sendFrame(g_chirpUsb, CAM_RES2_WIDTH, CAM_RES2_HEIGHT);
	else
		sendCustom();
	// resume streaming
	SM_OBJECT->currentLine = 0;
	SM_OBJECT->stream = 1; // resume streaming

	return 0;
}

uint32_t getRGB(uint16_t x, uint16_t y, uint8_t sat)
{
	uint32_t rgb;
	uint8_t r, g, b;
	uint16_t i, j, rsum, gsum, bsum, d;
	int16_t x0, x1, y0, y1;
	uint8_t *p = (uint8_t *)SRAM1_LOC + CAM_PREBUF_LEN;
	// average a square of size W
	
	if (x>=CAM_RES2_WIDTH)
		x = CAM_RES2_WIDTH-1;
	if (y>=CAM_RES2_HEIGHT)
		y = CAM_RES2_HEIGHT-1;
	
	x0 = x-g_rgbSize;
	if (x0<=0)
		x0 = 1;
	x1 = x+g_rgbSize;
	if (x1>=CAM_RES2_WIDTH)
		x1 = CAM_RES2_WIDTH-1;
	
	y0 = y-g_rgbSize;
	if (y0<=0)
		y0 = 1;
	y1 = y+g_rgbSize;
	if (y1>=CAM_RES2_HEIGHT)
		y1 = CAM_RES2_HEIGHT-1;
	
	for (i=y0, rsum=gsum=bsum=0; i<=y1; i++)
	{
		for (j=x0; j<=x1; j++)
		{
			interpolate(p, j, i, CAM_RES2_WIDTH, &r, &g, &b);
			rsum += r;
			gsum += g;
			bsum += b;
		}
	}
	d = (y1-y0+1)*(x1-x0+1);
	rsum /= d;
	gsum /= d;
	bsum /= d;
	
	rgb = rgbPack(r, g, b); 
	if (sat)
		return saturate(rgb);
	else
		return rgb;
}

static size_t strnlen(const char* data, size_t max){
	size_t len = 0;
	while(*data++ && len < max)
		len++;
	return len;
}

int ProgVideo::packet(uint8_t type, const uint8_t *data, uint8_t len, bool checksum)
{
	if (type==TYPE_REQUEST_GETRGB)
	{
		uint16_t x, y;
		uint32_t rgb;
		uint8_t saturate;

		if (len!=5)
		{
			ser_sendError(SER_ERROR_INVALID_REQUEST, checksum);
			return 0;
		}
		
		x = *(uint16_t *)(data+0);
		y = *(uint16_t *)(data+2);
		saturate = *(data+4);

		rgb = getRGB(x, y, saturate);
		ser_sendResult(rgb, checksum);
		
		return 0;
	} else if (type == TYPE_REQUEST_GETFRAMEBUF) {
		uint32_t start_addr;
		uint8_t req_len;

		if (len!=5)
		{
			ser_sendError(SER_ERROR_INVALID_REQUEST, checksum);
			return 0;
		}
		
		start_addr = *(uint32_t *)(data+0);
		req_len = *(uint8_t *)(data+4);
		
		if ((req_len > 249)||
			(req_len+start_addr > CAM_RES2_WIDTH * CAM_RES2_HEIGHT )){
				ser_sendError(SER_ERROR_INVALID_REQUEST, checksum);
				return 0;
			}

		uint8_t * txData;
		uint8_t * p = (uint8_t*)SRAM1_LOC + CAM_PREBUF_LEN;
		ser_getTx(&txData);
		memcpy(txData, p+start_addr, req_len);
		ser_setTx(TYPE_RESPONSE_GETFRAMEBUF, req_len, checksum);
		return 0;
	} 
#if USE_QRCODE
	else if (type == TYPE_REQUEST_QRDECODE) {
		if(!scanner){
			scanner = zbar_image_scanner_create();
			if(!scanner){
				ser_sendError(SER_ERROR_INVALID_REQUEST, checksum);
				return 0;
			}
			zbar_image_scanner_set_config(scanner, ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
		}
		
		SM_OBJECT->stream = 0; // pause after frame grab is finished
		
		uint8_t *cam_data = (uint8_t*)SRAM1_LOC + CAM_PREBUF_LEN;
		bayer2gray(cam_data, CAM_RES2_WIDTH, CAM_RES2_HEIGHT);
		
		zbar_image_t* image = zbar_image_create();
		zbar_image_set_format(image, *(int*)"Y800");
		zbar_image_set_size(image, CAM_RES2_WIDTH, CAM_RES2_HEIGHT);
		zbar_image_set_data(image, cam_data, CAM_RES2_WIDTH*CAM_RES2_HEIGHT, NULL);
		
		zbar_scan_image(scanner, image);
		
		SM_OBJECT->stream = 1; // resume
		
		uint8_t *txData, sendlen = 0;
		ser_getTx(&txData);

		const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
		for(;symbol;symbol=zbar_symbol_next(symbol)){
			int* typ = (int*)txData;
			uint8_t *len = (uint8_t*)txData;
			
			const char* data = zbar_symbol_get_data(symbol);
			uint8_t symbollen = strnlen(data, 0xf0-5);
			if(symbollen + 5 + sendlen > 0xf0)
				break;
			
			*typ = (int)zbar_symbol_get_type(symbol);
			*len = symbollen;
			memcpy(txData+1, data, symbollen);
			txData += symbollen+1;
			sendlen += symbollen+1;
		}
		ser_setTx(TYPE_RESPONSE_QRDECODE, sendlen, checksum);
		zbar_image_destroy(image);
		
		return 0;
	}
#endif
	
	// nothing rings a bell, return error
	return -1;
}


void ProgVideo::sendCustom(uint8_t renderFlags)
{
	uint32_t fourcc;

	// cooked mode
	if (g_execArg==1) 
		cam_sendFrame(g_chirpUsb, CAM_RES2_WIDTH, CAM_RES2_HEIGHT, RENDER_FLAG_FLUSH, FOURCC('C','M','V','2'));
	//  experimental mode, for new monmodules, etc.
	else if (100<=g_execArg && g_execArg<200) 
	{
		fourcc = FOURCC('E','X',((g_execArg%100)/10 + '0'), ((g_execArg%10) + '0'));
		cam_sendFrame(g_chirpUsb, CAM_RES2_WIDTH, CAM_RES2_HEIGHT, RENDER_FLAG_FLUSH, fourcc);
	}
	// undefined, just send plain raw frame (BA81)
	else 
		cam_sendFrame(g_chirpUsb, CAM_RES2_WIDTH, CAM_RES2_HEIGHT);

}




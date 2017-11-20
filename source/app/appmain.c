


#include "appmain.h"
#include "../gui/layouts/appgui.h"
#include "../sys/utils.h"
#include "../math/v3f.h"
#include "../math/matf.h"
#include "../math/3dmath.h"
#include "../sys/syswin.h"
#include "../gui/wg.h"
#include "../gui/gui.h"
#include "../sim/simdef.h"
#include "../sim/user.h"
#include "../sound/sound.h"
#include "../net/net.h"
#include "../gui/font.h"
#include "../sim/simflow.h"
#include "../sim/simvars.h"
#include "../render/shader.h"

char g_appmode = APPMODE_LOGO;
char g_viewmode = VIEWMODE_FIRST;
char g_restage = 0;

#define MX	0
#define MY	0
#define MZ -0.5/6371*384400
#define EX	0.1
#define EY	0
#define EZ	1.5
#define SX	0
#define SY	0
#define SZ	149600000/6371.0*0.5

#ifdef PLAT_WIN
HINSTANCE g_hinst = NULL;
#endif

void loadsysres()
{
	loadfonts();
	loadcursors();
}

void upload()
{
	wg *gui, *menu;

	gui = (wg*)&g_gui;

	switch(g_restage)
	{
	case 0:
/*		if(!loadqmod()) */ g_restage++;
		break;
	case 1:
		if(!loadqtex())
		{

	//		lfree(&g_modload);
			vfree(&g_texload);

//			g_lastLMd = -1;
			g_lastloadtex = -1;

			g_appmode = APPMODE_MENU;
			wghide(gui);
			menu = wgget(gui, "menu");
			wgshow(menu);
		}
		break;
	}
}

void upreload()
{
	wg *gui, *load;

	gui = (wg*)&g_gui;

	g_restage = 0;
	g_lastloadtex = -1;
//	g_lastLMd = -1;
	wgfree(gui);
//	FreeModels();
	freesps();
	freetexs();
	breakwin(APPTIT);
	makewin(APPTIT);

	/* Important - VBO only possible after window GL context made. */
	g_appmode = APPMODE_LOADING;

	loadsysres();
	queuesimres();
	makewg();

	wghide(gui);
	load = wgget(gui, "loading");
	wgshow(load);

	/*
	TODO
	Overhaul ui system
	Make scrollable everywhere
	Croppable pipeline 
	*/
}

void update()
{
//	if(g_sock)
//		UpdNet();

	switch(g_appmode)
	{
	case APPMODE_LOGO:
		uplogo();
		break;
//	case APPMODE_INTRO:
//		upintro();
//		break;
	case APPMODE_LOADING:
		upload();
		break;
	case APPMODE_RELOADING:
		upreload();
		break;
	case APPMODE_PLAY:
		upsim();
		break;
	case APPMODE_EDITOR:
//		uped();
		break;
	}
}

float rx = 0;
float rxx = 0;
float rxxx = 0;

v3f mvf()
{
	v3f mv;

	mv.x = MX;
	mv.y = MY;
	mv.z = MZ;

	mv = rot3f(mv, 2.0 * 3.14159 * rx, 0, 1, 0);

	mv.x += EX;
	mv.y += EY;
	mv.z += EZ;

	return mv;
}

void drawscene(float* proj, float* viewmat, float* modelmat, float* modelviewinv,
	float mvLightPos[3], float lightDir[3])
{
	static int ti = -1;
	static int t2 = -1;
	static int t3 = -1;
	mf mvp;
	glshader *s;
	v3f corner[4];
	v3f v;
	v3f u;
	v3f r;
	v3f mv;
	v3f srt, mft;
	v3f np;
	v3f ep;
	mf pp, vp;
	v3f vv;
	float ee;
	v3f npp;

	rx += 4.2314383254416208096062122801017 / 10000000 / 30.0f * g_drawiv * 30.0f;
	rxx += 4.2314383254416208096062122801017 / 10000000 / 30.0f * g_drawiv * 30.0f;

	//return;
	if (ti == -1)
		createtex(&(unsigned int)ti, "tex/1_earth_8k.jpg", dfalse, dfalse, dfalse);
	if (t2 == -1)
		createtex(&(unsigned int)t2, "tex/moon.jpg", dfalse, dfalse, dfalse);
	if (t3 == -1)
		createtex(&(unsigned int)t3, "tex/sun.jpg", dfalse, dfalse, dfalse);

		//createtex(&(unsigned int)ti, "tex/paper_grid_PNG5432.jpg", dfalse, dfalse, dfalse);
	//ti = 0;

	v3fsub(&v, g_camf.view, g_camf.pos);
	v = norm3f(v);
	//v.x = 0;
	//v.y = 0;
	//v.z = -2;
	//v3fmul(&v, v, -1.0f);

	u = cross3f(g_camf.strafe, v);
	//u = g_camf.up;
	r = g_camf.strafe;

	vv = g_camf.v;
	v3fmul(&vv, vv, g_drawiv * 30.0f);
	v3fadd(&np, g_camf.pos, vv);
	npp = np;

	ep.x = EX;
	ep.y = EY;
	ep.z = EZ;
	v3fsub(&np, np, ep);

	mv = mvf();
	v3fsub(&npp, npp, mv);

	if (mag3f(np) > 0.5f && mag3f(npp) > 0.5f * 1737 / 6371)
	{
		v3fadd(&g_camf.pos, g_camf.pos, vv);
		v3fadd(&g_camf.view, g_camf.view, vv);
		v3fsub(&np, ep, g_camf.pos);
		ee = mag3f(np);
		v3fmul(&np, np, 1.0 * 5.972 * 1000 * 6.67408 * g_drawiv * 30.0f / (ee*ee*ee / (0.5 / 6371) / (0.5 / 6371) / (0.5 / 6371)) / (6371 / 0.5));
		ee = mag3f(npp);
		v3fmul(&npp, npp, 1.0 * 7.3476730 * 10 * 6.67408 * g_drawiv * 30.0f / (ee*ee*ee / (0.5 / 6371) / (0.5 / 6371) / (0.5 / 6371)) / (6371 / 0.5));
		v3fadd(&g_camf.v, g_camf.v, np);
		v3fadd(&g_camf.v, g_camf.v, npp);
	}
	else
	{
		g_camf.v.x = 0;
		g_camf.v.y = 0;
		g_camf.v.z = 0;
	}
	v3fmul(&srt, g_camf.strafe, 100.0 / (30.0*60.0) / (6371 / 0.5) * g_drawiv * 30.0f);
	v3fmul(&mft, v, 100.0 / (30.0*60.0) / (6371 / 0.5) * g_drawiv * 30.0f);

	if (g_camf.r)	v3fadd(&g_camf.v, g_camf.v, srt);
	if (g_camf.b)	v3fsub(&g_camf.v, g_camf.v, mft);
	if (g_camf.l)	v3fsub(&g_camf.v, g_camf.v, srt);
	if (g_camf.f)	v3fadd(&g_camf.v, g_camf.v, mft);

	corner[0].x = -1.0f;
	corner[0].y = 1.0f;
	corner[0].z = 1.0f;

	corner[1].x = 1.0f;
	corner[1].y = 1.0f;
	corner[1].z = 1.0f;

	corner[2].x = 1.0f;
	corner[2].y = -1.0f;
	corner[2].z = 1.0f;

	corner[3].x = -1.0f;
	corner[3].y = -1.0f;
	corner[3].z = 1.0f;

	mv = mvf();

#if 0
	g_camf.v.x = 0;
	g_camf.v.y = 0;
	g_camf.v.z = 0;
	g_camf.pos = mv;
	g_camf.pos.x += 0.5f;
	g_camf.view = g_camf.pos;
	g_camf.view.x -= 1.0f;
	g_camf.strafe.x = 0;
	g_camf.strafe.y = 1;
	g_camf.strafe.z = 0;
	g_camf.up.x = 0;
	g_camf.up.y = 0;
	g_camf.up.z = -1;
	v3fsub(&g_camf.view, g_camf.view, g_camf.pos);
	g_camf.up = norm3f(cross3f(g_camf.strafe, g_camf.view));
	v3fadd(&g_camf.view, g_camf.view, g_camf.pos);
#endif

#if 0
	v3fsub(&g_camf.view, g_camf.view, g_camf.pos);
	g_camf.pos.x = 0;
	g_camf.pos.y = 0;
	g_camf.pos.z = (408 + 6371) / (6371 / 0.5);
	//g_camf.pos = rot3f(g_camf.pos, 92.0f * 60 * rxx / 40075.0 * 3.14159 * 2.0, 2.0 * 3.14159 * rx, 0, 1, 0);
	g_camf.pos = rot3f(g_camf.pos, 2.0 * 3.14159 * rx, 0, 1, 0);
	g_camf.pos.x += EX;
	g_camf.pos.y += EY;
	g_camf.pos.z += EZ;
	v3fadd(&g_camf.view, g_camf.view, g_camf.pos);
#endif

	pp = pproj(40.0f,
		(float)g_width / (float)g_height,
		0.1f, MAX_DISTANCE);
	//MAX_DISTANCE, 0.1f);

	vp = lookat(g_camf.view.x, g_camf.view.y, g_camf.view.z,
		g_camf.pos.x, g_camf.pos.y, g_camf.pos.z,
		g_camf.up.x, g_camf.up.y, g_camf.up.z);

	mfset((mf*)proj, (float*)&pp);
	mfset((mf*)viewmat, (float*)&vp);

	//mfset(&mvp, proj);
	//mfpostmult2(&mvp, (mf*)viewmat);
	//mfpostmult2(&mvp, (mf*)modelmat);
	mfset(&mvp, viewmat);
	mfpostmult(&mvp, (mf*)proj);
	//mfset(&mvp, proj);
	//mfpostmult(&mvp, (mf*)viewmat);
	usesh(SH_E);
	s = g_shader + g_cursh;
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, GL_FALSE, mvp.matrix);
	glEnable(GL_DEPTH_TEST);
	glUniform3f(s->slot[SSLOT_CAMCEN], g_camf.pos.x, g_camf.pos.y, g_camf.pos.z);
	//glUniform3f(s->slot[SSLOT_EP], 0, 0, -23073.300894679014283471982420342);
	glUniform3f(s->slot[SSLOT_EP], EX, EY, EZ);
	glUniform3f(s->slot[SSLOT_MP], mv.x, mv.y, mv.z);
	glUniform3f(s->slot[SSLOT_SP], SX, SY, SZ);
	glUniform3f(s->slot[SSLOT_CORNERA], corner[0].x, corner[0].y, corner[0].z);
	glUniform3f(s->slot[SSLOT_CORNERB], corner[1].x, corner[1].y, corner[1].z);
	glUniform3f(s->slot[SSLOT_CORNERC], corner[2].x, corner[2].y, corner[2].z);
	glUniform3f(s->slot[SSLOT_CORNERD], corner[3].x, corner[3].y, corner[3].z);
	glUniform3f(s->slot[SSLOT_RIGHT], r.x, r.y, r.z);
	glUniform3f(s->slot[SSLOT_UP], u.x, u.y, u.z);
	glUniform3f(s->slot[SSLOT_VIEW], v.x, v.y, v.z);
	glUniform1f(s->slot[SSLOT_WIDTH], g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], g_height);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_tex[ti].texname);
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);
	glVertexPointer(3, GL_FLOAT, 0, corner);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	endsh();
	usesh(SH_M);
	s = g_shader + g_cursh;
	glUniform1f(s->slot[SSLOT_RX], rx);
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, GL_FALSE, mvp.matrix);
	glUniform3f(s->slot[SSLOT_CAMCEN], g_camf.pos.x, g_camf.pos.y, g_camf.pos.z);
	//glUniform3f(s->slot[SSLOT_EP], 0, 0, -23073.300894679014283471982420342);
	glUniform3f(s->slot[SSLOT_EP], EX, EY, EZ);
	glUniform3f(s->slot[SSLOT_MP], mv.x, mv.y, mv.z);
	glUniform3f(s->slot[SSLOT_SP], SX, SY, SZ);
	glUniform3f(s->slot[SSLOT_CORNERA], corner[0].x, corner[0].y, corner[0].z);
	glUniform3f(s->slot[SSLOT_CORNERB], corner[1].x, corner[1].y, corner[1].z);
	glUniform3f(s->slot[SSLOT_CORNERC], corner[2].x, corner[2].y, corner[2].z);
	glUniform3f(s->slot[SSLOT_CORNERD], corner[3].x, corner[3].y, corner[3].z);
	glUniform3f(s->slot[SSLOT_RIGHT], r.x, r.y, r.z);
	glUniform3f(s->slot[SSLOT_UP], u.x, u.y, u.z);
	glUniform3f(s->slot[SSLOT_VIEW], v.x, v.y, v.z);
	glUniform1f(s->slot[SSLOT_WIDTH], g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], g_height);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_tex[t2].texname);
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);
	glVertexPointer(3, GL_FLOAT, 0, corner);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	endsh();
#if 02
	usesh(SH_S);
	s = g_shader + g_cursh;
	glUniform1f(s->slot[SSLOT_RX], rxxx);
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, GL_FALSE, mvp.matrix);
	glUniform3f(s->slot[SSLOT_CAMCEN], g_camf.pos.x, g_camf.pos.y, g_camf.pos.z);
	//glUniform3f(s->slot[SSLOT_EP], 0, 0, -23073.300894679014283471982420342);
	glUniform3f(s->slot[SSLOT_EP], EX, EY, EZ);
	glUniform3f(s->slot[SSLOT_MP], mv.x, mv.y, mv.z);
	glUniform3f(s->slot[SSLOT_SP], SX, SY, SZ);
	glUniform3f(s->slot[SSLOT_CORNERA], corner[0].x, corner[0].y, corner[0].z);
	glUniform3f(s->slot[SSLOT_CORNERB], corner[1].x, corner[1].y, corner[1].z);
	glUniform3f(s->slot[SSLOT_CORNERC], corner[2].x, corner[2].y, corner[2].z);
	glUniform3f(s->slot[SSLOT_CORNERD], corner[3].x, corner[3].y, corner[3].z);
	glUniform3f(s->slot[SSLOT_RIGHT], r.x, r.y, r.z);
	glUniform3f(s->slot[SSLOT_UP], u.x, u.y, u.z);
	glUniform3f(s->slot[SSLOT_VIEW], v.x, v.y, v.z);
	glUniform1f(s->slot[SSLOT_WIDTH], g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], g_height);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_tex[t3].texname);
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);
	glVertexPointer(3, GL_FLOAT, 0, corner);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	endsh();
#endif

	glDisable(GL_DEPTH_TEST);
	endsh();
	flatview(g_width, g_height, 1, 1, 1, 1);
}

void drawscenedepth()
{
}

void makefbo(unsigned int* rendertex, unsigned int* renderrb, unsigned int* renderfb, unsigned int* renderdepthtex, int w, int h)
{
	/* OpenGL 1.4 way */
	GLenum DrawBuffers[2];

	glGenTextures(1, rendertex);
	glBindTexture(GL_TEXTURE_2D, *rendertex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, renderdepthtex);
	glBindTexture(GL_TEXTURE_2D, *renderdepthtex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, renderfb);
	glBindFramebuffer(GL_FRAMEBUFFER, *renderfb);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *renderdepthtex, 0);

	DrawBuffers[0] = GL_COLOR_ATTACHMENT0;
	DrawBuffers[1] = GL_DEPTH_ATTACHMENT;
	glDrawBuffers(1, DrawBuffers);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		errm("Error", "Couldn't create framebuffer for render.");
		return;
	}
}

void delfbo(unsigned int* rendertex, unsigned int* renderrb, unsigned int* renderfb, unsigned int* renderdepthtex)
{
	/* delete resources */
	glDeleteTextures(1, rendertex);
	glDeleteTextures(1, renderdepthtex);
	/* Bind 0, which means render to back buffer, as a result, fb is unbound */
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, renderfb);
	CHECKGL();
}

void draw()
{
	char m[1230];
	float white[4] = {1,1,1,1};
	float frame[4] = {0,g_height/3,(float)g_width,(float)g_height};
	wg *gui;
	v3f mv;
	//mf proj, view, model, mvinv;
	//float lpos[3], ldir[3];

	gui = (wg*)&g_gui;

	/* TODO leave as float for now then use fixmath int's */

	flatview(g_width, g_height, 1, 1, 1, 1);
	glDisable(GL_DEPTH_TEST);

	//drawim(g_tex[0].texname,
	//	0,0,100,100, 
	//	0,0,1,1,
	//	frame);
	if (g_appmode == APPMODE_PLAY)
	{
		mf proj, view, model, mvinv;
		v3f lpos, ldir;

		proj = pproj(40.0f,
			(float)g_width / (float)g_height,
			0.1f, MAX_DISTANCE);
			//MAX_DISTANCE, 0.1f);

		view = lookat(g_camf.view.x, g_camf.view.y, g_camf.view.z,
			g_camf.pos.x, g_camf.pos.y, g_camf.pos.z,
			g_camf.up.x, g_camf.up.y, g_camf.up.z);

		mfreset(&model);

		drawscene(proj.matrix,
			view.matrix,
			model.matrix,
			mvinv.matrix,
			(float*)&lpos, (float*)&ldir);
	}

	mv = mvf();

	sprintf(m, "%f,%f,%f kmph    \r\n%f,%f,%f km  \r\nview %f,%f,%f \r\ned%f \r\nfps%f \r\nmd%f \r\nsd%f",
		g_camf.v.x * (6371 / 0.5) * 30.0f * 60.0f * 60,
		g_camf.v.y * (6371 / 0.5) * 30.0f * 60.0f * 60,
		g_camf.v.z * (6371 / 0.5) * 30.0f * 60.0f * 60,
		g_camf.pos.x * (6371 / 0.5),
		g_camf.pos.y * (6371 / 0.5),
		g_camf.pos.z * (6371 / 0.5),
		(g_camf.view.x - g_camf.pos.x),
		(g_camf.view.y - g_camf.pos.y),
		(g_camf.view.z - g_camf.pos.z),
		sqrtf( (EX-g_camf.pos.x)*(EX - g_camf.pos.x) + (EY - g_camf.pos.y)*(EY - g_camf.pos.y) + (EZ - g_camf.pos.z)*(EZ - g_camf.pos.z) ) * (6371 / 0.5) - 6371,
		(float)g_indrawfps,
		sqrtf((mv.x - g_camf.pos.x)*(mv.x - g_camf.pos.x) + (mv.y - g_camf.pos.y)*(mv.y - g_camf.pos.y) + (mv.z - g_camf.pos.z)*(mv.z - g_camf.pos.z)) * (6371 / 0.5) - 1737,
		sqrtf((SX - g_camf.pos.x)*(SX - g_camf.pos.x) + (SY - g_camf.pos.y)*(SY - g_camf.pos.y) + (SZ - g_camf.pos.z)*(SZ - g_camf.pos.z)) * (6371 / 0.5) - 695700
		);
	drawt(MAINFONT8, frame, frame, m,
	white, 0, -1, dfalse, dtrue);

	wgframeup(gui);
	wgdraw(gui);

	endsh();
	glEnable(GL_DEPTH_TEST);
}

void loadcfg()
{
	lnode *rit; /* v2i */
	v2i *rp;
	int w, h;
	char cfgfull[DMD_MAX_PATH+1];
	char line[128];
	char key[128];
	char act[128];
	FILE *fp;
	float valuef;
	int valuei;
	dbool valueb;
	//int i;

	enumdisp();

	if(g_ress.size)
	{
		rit = g_ress.head;
		rp = (v2i*)rit->data;
		g_selres = *rp;
	}
	else
	{
		SDL_GL_GetDrawableSize(g_win, &w, &h);

		g_selres.x = w;
		g_selres.y = h;
	}

	for(rit=g_ress.head; rit; rit=rit->next)
	{
		/* below acceptable height? */
		if(g_selres.y < 480)
		{
			rp = (v2i*)rit->data;

			if(rp->y > g_selres.y &&
				rp->x > rp->y)
			{
				g_selres = *(v2i*)rit->data;
			}
		}
		/* already of acceptable height? */
		else
		{
			rp = (v2i*)rit->data;
			//get smallest acceptable resolution
			if(rp->x < g_selres.y &&
				rp->x > rp->y)
			{
				g_selres = *(v2i*)rit->data;
			}

			break;
		}
	}

	//SwitchLang(LANG_ENG);

	fullwrite(CFGFILE, cfgfull);

	fp = fopen(cfgfull, "r");

	if(!fp)
		return;

	while(!feof(fp))
	{
		fgets(line, 127, fp);

		if(strlen(line) > 127)
			continue;

		act[0] = 0;
		key[0] = 0;

		if(sscanf(line, "%s %s", key, act) < 2)
			continue;

		sscanf(act, "%f", &valuef);
		valuei = (int)valuef;
		valueb = (dbool)valuef;

		if(strcmp(key, "fullscreen") == 0)					g_fs = valueb;
		else if(strcmp(key, "client_width") == 0)			g_width = g_selres.x = valuei;
		else if(strcmp(key, "client_height") == 0)			g_height = g_selres.y = valuei;
		else if(strcmp(key, "screen_bpp") == 0)				g_bpp = valuei;
//		else if(strcmp(key, "volume") == 0)					SetVol(valuei);
//		else if(strcmp(key, "language") == 0)				SwitchLang(GetLang(act));
	}

	fclose(fp);
}

void loadname()
{
	char cfgfull[DMD_MAX_PATH+1];
	FILE *fp;

	fullwrite("name.txt", cfgfull);
	fp = fopen(cfgfull, "r");

	if(!fp)
	{
		//GenName(g_name);
		sprintf(g_name, "User%d", (int)(rand()%1000));
		return;
	}

	fgets(g_name, MAXNAME, fp);
	fclose(fp);
}

void writecfg()
{
	char cfgfull[DMD_MAX_PATH+1];
	FILE* fp = fopen(cfgfull, "w");
	fullwrite(CFGFILE, cfgfull);
	if(!fp)
		return;
	fprintf(fp, "fullscreen %d \r\n\r\n", g_fs ? 1 : 0);
	fprintf(fp, "client_width %d \r\n\r\n", g_selres.x);
	fprintf(fp, "client_height %d \r\n\r\n", g_selres.y);
	fprintf(fp, "screen_bpp %d \r\n\r\n", g_bpp);
	//fprintf(fp, "volume %d \r\n\r\n", g_volume);
	//fprintf(fp, "language %s\r\n\r\n", g_lang);
	fclose(fp);
}

void writename()
{
	char cfgfull[DMD_MAX_PATH+1];
	FILE* fp = fopen(cfgfull, "w");
	fullwrite("name.txt", cfgfull);
	if(!fp)
		return;
	fprintf(fp, "%s", g_name);
	fclose(fp);
}

/* Define the function to be called when ctrl-c (SIGINT) signal is sent to process */
void sigcall(int signum)
{
	exit(0);
}

void appinit()
{
	char msg[128];
	//SDL_version compile_version;
	//SDL_version *link_version;
	//int flags;
	//int initted;
	char full[DMD_MAX_PATH+1];

	fullpath("log.txt", full);
	g_applog = fopen(full, "wb");

#ifdef PLAT_LINUX
	signal(SIGINT, sigcall);
#endif

	if(SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		sprintf(msg, "SDL_Init: %s\n", SDL_GetError());
		errm("Error", msg);
	}

#if 0
	if(SDLNet_Init() == -1)
	{
		sprintf(msg, "SDLNet_Init: %s\n", SDLNet_GetError());
		errm("Error", msg);
	}
#endif

#if 0
	link_version=(SDL_version*)Mix_Linked_Version();
	SDL_MIXER_VERSION(&compile_version);
	printf("compiled with SDL_mixer version: %d.%d.%d\n",
		compile_version.major,
		compile_version.minor,
		compile_version.patch);
	printf("running with SDL_mixer version: %d.%d.%d\n",
		link_version->major,
		link_version->minor,
		link_version->patch);

	// load support for the OGG and MOD sample/music formats
	flags=MIX_INIT_OGG|MIX_INIT_MP3;
	initted=Mix_Init(flags);
	if( (initted & flags) != flags)
	{
		sprintf(msg, "Mix_Init: Failed to init required ogg and mod support!\nMix_Init: %s", Mix_GetError());
		/* errm("Error", msg); */
	}

	if(SDL_Init(SDL_INIT_AUDIO)==-1) {
		sprintf(msg, "SDL_Init: %s\n", SDL_GetError());
		errm("Error", msg);
	}
	/* show 44.1KHz, signed 16bit, system byte order,
	      stereo audio, using 1024 byte chunks */
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)==-1)
	{
		printf("Mix_OpenAudio: %s\n", Mix_GetError());
		errm("Error", msg);
	}

	Mix_AllocateChannels(SCHANS);
#endif

	if(!g_applog)
		openlog("log.txt", APPVER);

	srand((unsigned int)getticks());

	vinit(&g_texload, sizeof(textoload));

	/* TODO c-style inits, not constructors */
	loadcfg();
	loadname();
//	MapKeys();
}

void appdeinit()
{
//	lnode *cit;
	wgg* gui;
	unsigned __int64 start;

//	endsess();
//	FreeMap();

	gui = &g_gui;
	wgfree((wg*)gui);

	breakwin(APPTIT);

//	for(cit=g_cn.head; cit; cit=cit->next)
//	{
//		Disconnect((NetConn*)cit->data);
//	}

	start = getticks();
	/* After quit, wait to send out quit packet to make sure host/clients recieve it. */
	while (getticks() - start < QUIT_DELAY)
	{
//		if(NetQuit())
//			break;
//		if(g_sock)
//			UpdNet();
	}

//	if(g_sock)
//	{
//		SDLNet_UDP_Close(g_sock);
//		g_sock = NULL;
//	}

//	lfree(&g_cn);

//	FreeSounds();
	//Mix_CloseAudio();
	//Mix_Quit();
	//SDLNet_Quit();
	SDL_Quit();
}

int evproc(void *userdata, SDL_Event *e)
{
	wg *gui;
	inev ie;
	v2i old;

	gui = (wg*)&g_gui;

	ie.intercepted = dfalse;
	ie.curst = CU_DEFAULT;

	switch(e->type)
	{
		case SDL_QUIT:
			g_quit = dtrue;
			break;
		case SDL_KEYDOWN:
			ie.type = INEV_KEYDOWN;
			ie.key = e->key.keysym.sym;
			ie.scancode = e->key.keysym.scancode;
			CHECKGL();

			/* Handle copy */
			if( e->key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
			{
				ie.type = INEV_COPY;
			}
			/* Handle paste */
			if( e->key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
			{
				ie.type = INEV_PASTE;
			}
			/* Select all */
			if( e->key.keysym.sym == SDLK_a && SDL_GetModState() & KMOD_CTRL )
			{
				ie.type = INEV_SELALL;
			}

			CHECKGL();
			wgin(gui, &ie);
			CHECKGL();

			if(!ie.intercepted)
				g_keys[e->key.keysym.scancode] = dtrue;

			g_keyin = ie.intercepted;
			break;
		case SDL_KEYUP:
			ie.type = INEV_KEYUP;
			ie.key = e->key.keysym.sym;
			ie.scancode = e->key.keysym.scancode;

			CHECKGL();
			wgin(gui, &ie);
			CHECKGL();

			if(!ie.intercepted)
				g_keys[e->key.keysym.scancode] = dfalse;

			g_keyin = ie.intercepted;
			break;
		case SDL_TEXTINPUT:
			/* UTF8 */
			ie.type = INEV_TEXTIN;
			strcpy(ie.text, e->text.text);

			CHECKGL();
			wgin(gui, &ie);
			CHECKGL();
			break;

		case SDL_MOUSEWHEEL:
			ie.type = INEV_MOUSEWHEEL;
			ie.amount = e->wheel.y;

			CHECKGL();
				wgin(gui, &ie);
			CHECKGL();
			break;
		case SDL_MOUSEBUTTONDOWN:
			switch (e->button.button)
			{
			case SDL_BUTTON_LEFT:
				g_mousekeys[MOUSE_LEFT] = dtrue;

				ie.type = INEV_MOUSEDOWN;
				ie.key = MOUSE_LEFT;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;

				CHECKGL();
				wgin(gui, &ie);
				CHECKGL();

				g_keyin = ie.intercepted;
				break;
			case SDL_BUTTON_RIGHT:
				g_mousekeys[MOUSE_RIGHT] = dtrue;

				ie.type = INEV_MOUSEDOWN;
				ie.key = MOUSE_RIGHT;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;

				CHECKGL();
				wgin(gui, &ie);
				CHECKGL();
				break;
			case SDL_BUTTON_MIDDLE:
				g_mousekeys[MOUSE_MIDDLE] = dtrue;

				ie.type = INEV_MOUSEDOWN;
				ie.key = MOUSE_MIDDLE;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;

				CHECKGL();
				wgin(gui, &ie);
				CHECKGL();
				break;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			switch (e->button.button)
			{
			case SDL_BUTTON_LEFT:
				g_mousekeys[MOUSE_LEFT] = dfalse;

				ie.type = INEV_MOUSEUP;
				ie.key = MOUSE_LEFT;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;

				CHECKGL();
				wgin(gui, &ie);
				CHECKGL();
				break;
			case SDL_BUTTON_RIGHT:
				g_mousekeys[MOUSE_RIGHT] = dfalse;

				ie.type = INEV_MOUSEUP;
				ie.key = MOUSE_RIGHT;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;

				CHECKGL();
				wgin(gui, &ie);
				CHECKGL();
				break;
			case SDL_BUTTON_MIDDLE:
				g_mousekeys[MOUSE_MIDDLE] = dfalse;

				ie.type = INEV_MOUSEUP;
				ie.key = MOUSE_MIDDLE;
				ie.amount = 1;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;

				CHECKGL();
				wgin(gui, &ie);
				CHECKGL();
				break;
			}
			break;
		case SDL_MOUSEMOTION:

			if(g_mouseout)
			{
				g_mouseout = dfalse;
			}

			old = g_mouse;

			if(mousepos())
			{
				ie.type = INEV_MOUSEMOVE;
				ie.x = g_mouse.x;
				ie.y = g_mouse.y;
				ie.dx = g_mouse.x - old.x;
				ie.dy = g_mouse.y - old.y;

				CHECKGL();
				wgin(gui, &ie);
				CHECKGL();

				g_curst = ie.curst;
			}
			break;
	}

	return 0;
}

void evloop()
{
	SDL_Event e;

	CHECKGL();
	while (!g_quit)
	{
		CHECKGL();
		while (SDL_PollEvent(&e))
		{
			evproc(NULL, &e);
		}
		CHECKGL();

		if(g_quit)
			break;

		if ( !g_bg &&
			( (g_appmode == APPMODE_LOADING || g_appmode == APPMODE_RELOADING) || drawnext() ) )
		{
			CHECKGL();

			calcdrawrate();

			CHECKGL();
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			draw();
			CHECKGL();
			SDL_GL_SwapWindow(g_win);
			CHECKGL();

			CHECKGL();
		}

		if((g_appmode == APPMODE_LOADING || g_appmode == APPMODE_RELOADING) || upnext() )
		{
			calcuprate();
			update();
		}

		CHECKGL();
	}
}

#ifdef USESTEAM
//-----------------------------------------------------------------------------
// Purpose: callback hook for debug text emitted from the Steam API
//-----------------------------------------------------------------------------
extern "C" void __cdecl SteamAPIDebugTextHook( int nSeverity, const char *pchDebugText )
{
	// if you're running in the debugger, only warnings (nSeverity >= 1) will be sent
	// if you add -debug_steamapi to the command-line, a lot of extra informational messages will also be sent
#ifdef PLAT_WIN
	::OutputDebugString( pchDebugText );
#endif

	if(!g_applog)
		openlog("log.txt", APPVER);

	Log(pchDebugText);


	if ( nSeverity >= 1 )
	{
		// place to set a breakpoint for catching API errors
		int x = 3;
		x = x;
	}
}
#endif

void appmain()
{
	//*((int*)0) = 0;

#ifdef USESTEAM

	if ( SteamAPI_RestartAppIfNecessary( k_uAppIdInvalid ) )
	{
		// if Steam is not running or the game wasn't started through Steam, SteamAPI_RestartAppIfNecessary starts the 
		// local Steam client and also launches this game again.

		// Once you get a public Steam AppID assigned for this game, you need to replace k_uAppIdInvalid with it and
		// removed steaappid.txt from the game depot.

		return;
	}

	// appinit Steam CEG
	if ( !Steamworks_InitCEGLibrary() )
	{
#ifdef PLAT_WIN
		OutputDebugString( "Steamworks_InitCEGLibrary() failed\n" );
#endif
		errm( "Fatal Error", "Steam must be running to play this game (InitDrmLibrary() failed).\n" );
		return;
	}

	// Initialize SteamAPI, if this fails we bail out since we depend on Steam for lots of stuff.
	// You don't necessarily have to though if you write your code to check whether all the Steam
	// interfaces are NULL before using them and provide alternate paths when they are unavailable.
	//
	// This will also load the in-game steam overlay dll into your process.  That dll is normally
	// injected by steam when it launches games, but by calling this you cause it to always load,
	// even when not launched via steam.
	if ( !SteamAPI_Init() )
	{
#ifdef PLAT_WIN
		OutputDebugString( "SteamAPI_Init() failed\n" );
#endif
		errm( "Fatal Error", "Steam must be running to play this game (SteamAPI_Init() failed).\n" );
		return;
	}

	// set our debug handler
	SteamClient()->SetWarningMessageHook( &SteamAPIDebugTextHook );

#endif

	appinit();

	makewin(APPTIT);

	//SDL_ShowCursor(dfalse);
	loadsysres();
	queuesimres();
	makewg();

	evloop();

	appdeinit();
	//SDL_ShowCursor(dtrue);
}

dbool runops(const char* cmdline)
{
	if(strcmp(cmdline, "") == 0)
	{
//		strcpy(g_startmap, "");

		return dfalse;
	}
	else
	{
		/*
		TODO c90
		std::string cmdlinestr(cmdline);
		std::string find("+devmap ");
		int found = cmdlinestr.rfind(find);

		if(found != std::string::npos)
		{
			strcpy(g_startmap, "");

			startmap = cmdlinestr.psubstr(found+find.length(), cmdlinestr.length()-found-find.length());

			fprintf(g_applog, "%s\r\n", cmdline);
			fprintf(g_applog, "%s\r\n", startmap);

			//LoadMap(startmap.c_str());
			g_startmap = startmap;
		}
		*/
	}

	return dfalse;
}

#ifdef PLAT_WIN
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char* argv[])
#endif
{
  //  SDL_SetMainReady();

#ifdef PLAT_WIN
	g_hinst = hInstance;
#endif

#ifdef PLAT_WIN
	//runops(lpCmdLine);
#else
	char cmdline[DMD_MAX_PATH+124];
	cmdline[0] = 0;
	for(int ai=0; ai<argc; ai++)
	{
		strcat(cmdline, argv[ai]);

		if(ai+1<argc)
			strcat(cmdline, " ");
	}
	runops(cmdline);
#endif

#ifdef PLAT_WIN
	if ( IsDebuggerPresent() )
	{
		// We don't want to mask exceptions (or report them to Steam!) when debugging.
		// If you would like to step through the exception handler, attach a debugger
		// after running the game outside of the debugger.	

		appmain();
		return 0;
	}
#endif

#ifdef PLAT_WIN
#ifdef USESTEAM
	_set_se_translator( MiniDumpFunction );

	try  // this try block allows the SE translator to work
	{
#endif
#endif
		appmain();
#ifdef PLAT_WIN
#ifdef USESTEAM
	}
	catch( ... )
	{
		return -1;
	}
#endif
#endif

	return 0;
}

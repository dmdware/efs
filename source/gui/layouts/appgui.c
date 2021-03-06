









#include "../../app/appmain.h"
#include "../gui.h"
#include "../widgets/image.h"
#include "../widgets/button.h"
#include "../vl.h"
#include "../widgets/link.h"
#include "vpwrap.h"
#include "../widgets/vp.h"
#include "../../math/3dmath.h"

char g_lastsave[DMD_MAX_PATH+1];

static char g_restage = 0;

void skiplogo()
{
	wg *gui, *load;

	if(g_appmode != APPMODE_LOGO)
		return;

	g_appmode = APPMODE_LOADING;return;
	gui = (wg*)&g_gui;

	wghide( gui );
	load = wgget( gui, "loading" );
	wgshow( load );
}

void uplogo()
{
	wg *gui, *logo;
	imw *logo2;
	float a;
	static int stage = 0;

	skiplogo();
	return;

	gui = (wg*)&g_gui;

	if(stage < 60)
	{
		a = (float)stage / 60.0f;
		logo = wgget( gui, "logo");
		logo2 = (imw*)wgget( logo, "logo");
		logo2->rgba[3] = a;
	}
	else if(stage < 120)
	{
		a = 1.0f - (float)(stage-60) / 60.0f;
		logo = wgget( gui, "logo");
		logo2 = (imw*)wgget( logo, "logo");
		logo2->rgba[3] = a;
	}
	else
		skiplogo();

	stage++;
}

void szfs(wg* w)
{
	w->pos[0] = 0;
	w->pos[1] = 0;
	w->pos[2] = (float)g_width;
	w->pos[3] = (float)g_height;
}

void szfssq(wg* w)
{
	float minsz;
	
	minsz = minf((float)g_width-1, (float)g_height-1);

	w->pos[0] = g_width/2.0f - minsz/2.0f;
	w->pos[1] = g_height/2.0f - minsz/2.0f;
	w->pos[2] = g_width/2.0f + minsz/2.0f;
	w->pos[3] = g_height/2.0f + minsz/2.0f;
}

void szapplogo(wg* w)
{
	w->pos[0] = 30;
	w->pos[1] = 30;
	w->pos[2] = 200;
	w->pos[3] = 200;
}

void szapptit(wg* w)
{
	w->pos[0] = 30;
	w->pos[1] = 30;
	w->pos[2] = (float)g_width-1;
	w->pos[3] = 100;
}

void szbut(wg* w)
{
	w->pos[0] = 30;
	w->pos[1] = 30;
	w->pos[2] = 200;
	w->pos[3] = 100;
	cenlab((bwg*)w);
}

void cng()
{
	wg *w;
	w = (wg*)&g_gui;
	wghideall(w);
	g_appmode = APPMODE_PLAY;
	g_camf.pos.x = 0;
	g_camf.pos.y = 0.0;// 0.13632082875529744153194161042223;
	g_camf.pos.z = 0;
	g_camf.view.x = 0;
	g_camf.view.y = 0.0;// 0.13632082875529744153194161042223;
	g_camf.view.z = 1;
	g_camf.up.x = 0;
	g_camf.up.y = 1;
	g_camf.up.z = 0;
	g_camf.strafe.x = 1;
	g_camf.strafe.y = 0;
	g_camf.strafe.z = 0;
	g_camf.f = dfalse;
	g_camf.b = dfalse;
	g_camf.l = dfalse;
	g_camf.r = dfalse;
	g_camf.v.x = 1/(30.0*60.0)/(6371/0.5);
	g_camf.v.y = 0;
	g_camf.v.z = 1 / (30.0*60.0) / (6371 / 0.5);
}

void clg()
{
}

void cop()
{
}

void ccs()
{
}

void cqu()
{
	g_quit = dtrue;
}

void ced()
{
	wg* gui;
	wg* menu;
	wg* edm;

	gui = (wg*)&g_gui;
	menu = wgget(gui, "menu");
	edm = wgget(gui, "ed");

	wghide(menu);
	wgshow(edm);

	g_appmode = APPMODE_EDITOR;
}

void cjg()
{
}

void chg()
{
}

void ces()
{
	wg* gui;
	wg* menu;
	wg* edm;
	switch (g_appmode)
	{
	case APPMODE_PLAY:
		gui = (wg*)&g_gui;
		menu = wgget(gui, "menu");
		wgshow(menu);
		g_appmode = APPMODE_MENU;
		break;
	case APPMODE_EDITOR:
		gui = (wg*)&g_gui;
		menu = wgget(gui, "menu");
		edm = wgget(gui, "ed");

		wghide(edm);
		wgshow(menu);

		g_appmode = APPMODE_MENU;
		break;
	case APPMODE_MENU:
		gui = (wg*)&g_gui;
		menu = wgget(gui, "menu");

		if (menu->hidden)
		{
			edm = wgget(gui, "ed");
			wghide(edm);
			wgshow(menu);
		}
		else
		{
			g_quit = dtrue;
		}
		break;
	}
}

void esz(wg* bw)
{
	bw->pos[0] = g_width / 6 - 10;
	bw->pos[1] = g_height / 2 - 120 + *(int*)bw->extra * 22;
	bw->pos[2] = bw->pos[0] + 100;
	bw->pos[3] = bw->pos[1] + 22;
}

void mm(inev* ie)
{
	v3f v, u;
	v3f nv, nu, nr;

	v3fsub(&v, g_camf.view, g_camf.pos);
	v = norm3f(v);
	u = cross3f(g_camf.strafe, v);

	nv = rot3f(v, -ie->dy / 20.0f, g_camf.strafe.x, g_camf.strafe.y, g_camf.strafe.z);
	nv = rot3f(nv, -ie->dx / 20.0f, u.x, u.y, u.z);

	nu = rot3f(u, -ie->dy / 20.0f, g_camf.strafe.x, g_camf.strafe.y, g_camf.strafe.z);

	nr = rot3f(g_camf.strafe, -ie->dx / 20.0f, u.x, u.y, u.z);

	g_camf.strafe = nr;
	//g_camf.up = nu;
	g_camf.up = norm3f( cross3f(nr, nv) );
	nv = norm3f(cross3f(nu, nr));
	v3fadd(&g_camf.view, nv, g_camf.pos);
}

void mfb()
{
	g_camf.f = dtrue;
}
void mfub()
{
	g_camf.f = dfalse;
}

void mbb()
{
	g_camf.b = dtrue;
}
void mbub()
{
	g_camf.b = dfalse;
}

void mlb()
{
	g_camf.l = dtrue;
}
void mlub()
{
	g_camf.l = dfalse;
}

void mrb()
{
	g_camf.r = dtrue;
}
void mrub()
{
	g_camf.r = dfalse;
}

void efree(wg* bw)
{
	free(bw->extra);
	bw->extra = NULL;
}

void makewg()
{
	wgg *gui;
	vl *logo, *loading, *menu;
	bwg *but;
	imw *bg, *bg2;
	hpl *ng, *lg, *jg, *hg, *op, *cs, *qu, *ed;
	int *i[8], j;
	vl *edm;
	vp *edv[4];

	for (j = 0; j < 8; ++j)
	{
		i[j] = (int*)malloc(sizeof(int));
		*(i[j]) = j;
	}
	gui = &g_gui;
	wgginit(gui);

	gui->keydownfunc[SDL_SCANCODE_ESCAPE] = ces;
	gui->mmovef = mm;
	gui->keydownfunc[SDL_SCANCODE_W] = mfb;
	gui->keyupfunc[SDL_SCANCODE_W] = mfub;
	gui->keydownfunc[SDL_SCANCODE_S] = mbb;
	gui->keyupfunc[SDL_SCANCODE_S] = mbub;
	gui->keydownfunc[SDL_SCANCODE_A] = mlb;
	gui->keyupfunc[SDL_SCANCODE_A] = mlub;
	gui->keydownfunc[SDL_SCANCODE_D] = mrb;
	gui->keyupfunc[SDL_SCANCODE_D] = mrub;

	logo = (vl*)malloc(sizeof(vl));
	loading = (vl*)malloc(sizeof(vl));
	menu = (vl*)malloc(sizeof(vl));
	but = (bwg*)malloc(sizeof(bwg));
	bg = (imw*)malloc(sizeof(imw));
	ng = (hpl*)malloc(sizeof(hpl));
	lg = (hpl*)malloc(sizeof(hpl));
	jg = (hpl*)malloc(sizeof(hpl));
	hg = (hpl*)malloc(sizeof(hpl));
	op = (hpl*)malloc(sizeof(hpl));
	cs = (hpl*)malloc(sizeof(hpl));
	qu = (hpl*)malloc(sizeof(hpl));
	ed = (hpl*)malloc(sizeof(hpl));
	edm = (vl*)malloc(sizeof(vl));
	bg2 = (imw*)malloc(sizeof(imw));
	for (j = 0; j<4; ++j)
		edv[j] = (vp*)malloc(sizeof(vp));

	vwinit(logo, "logo", (wg*)gui);
	vwinit(loading, "loading", (wg*)gui);
	vwinit(menu, "menu", (wg*)gui);
	imwinit(bg, menu, "",
		"gui/bg.jpg", dtrue,
		szfs, 
		1, 1, 1, 1,
		0, 0, 1, 1);
	bwginit(but, menu, "",
		"", "label", "tooltip",
		MAINFONT16, BUST_LINEBASED,
		szbut, NULL, NULL,
		NULL, NULL,
		NULL, -1, NULL);
	hplinit(ng, menu, "", "new game", MAINFONT16, esz, cng, i[0], efree);
	hplinit(lg, menu, "", "load game", MAINFONT16, esz, clg, i[1], efree);
	hplinit(jg, menu, "", "join game", MAINFONT16, esz, cjg, i[2], efree);
	hplinit(hg, menu, "", "host game", MAINFONT16, esz, chg, i[3], efree);
	hplinit(op, menu, "", "options", MAINFONT16, esz, cop, i[4], efree);
	hplinit(cs, menu, "", "credits", MAINFONT16, esz, ccs, i[5], efree);
	hplinit(qu, menu, "", "quit", MAINFONT16, esz, cqu, i[6], efree);
	hplinit(ed, menu, "", "editor", MAINFONT16, esz, ced, i[7], efree);
	vwinit(edm, "ed", (wg*)gui);
	imwinit(bg2, edm, "",
		"gui/bg.jpg", dtrue,
		szfs,
		1, 1, 1, 1,
		0, 0, 1, 1);
	for(j=0; j<4; ++j)
		vpinit(edv[j], edm, "vp", szfp, fpdraw, 
			NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &g_fpi[j], NULL);

	wgadd((wg*)gui, (wg*)logo);
	wgadd((wg*)gui, (wg*)loading);
	wgadd((wg*)gui, (wg*)menu);
	wgadd((wg*)menu, (wg*)bg);
	wgadd((wg*)menu, (wg*)but);
	wgadd((wg*)menu, (wg*)ng);
	wgadd((wg*)menu, (wg*)lg);
	wgadd((wg*)menu, (wg*)jg);
	wgadd((wg*)menu, (wg*)hg);
	wgadd((wg*)menu, (wg*)op);
	wgadd((wg*)menu, (wg*)cs);
	wgadd((wg*)menu, (wg*)qu);
	wgadd((wg*)menu, (wg*)ed);
	wgadd((wg*)gui, (wg*)edm);
	wgadd((wg*)edm, (wg*)bg2);
	for (j = 0; j<4; ++j)
		wgadd((wg*)edm, (wg*)edv[j]);

	wghide((wg*)but);

	wghideall((wg*)gui);
	wgshow((wg*)logo);
}
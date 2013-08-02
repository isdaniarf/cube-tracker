#include "opencv2/opencv.hpp"
#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <Windows.h>
#include <time.h>


#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <assert.h>
#include  <signal.h>
extern "C" {
#include "cubedefs.h"
}

extern "C" char moveSolution[100];

static char manString[256];
extern "C" {
int subOptLev;
int symRed;
}
CubieCube cc_a;

void interseg(CvPoint*p1, CvPoint*p2, CvPoint*q1, CvPoint*q2, bool &succses, double &ta, double &tb, double &xx, double &yy)
{
	double x1 = p1[0].x;
	double x2 = p2[0].x;
	double x3 = q1[0].x;
	double x4 = q2[0].x;
	double y1 = p1[0].y;
	double y2 = p2[0].y;
	double y3 = q1[0].y;
	double y4 = q2[0].y;
	double den= ((y4-y3)*(x2-x1))-((x4-x3)*(y2-y1));
	if (abs(den)<0.1)
		{
			succses = FALSE;
			ta = 0;tb = 0;
			xx = 0;yy = 0;
		}
		else {
		double ua,ub;
		ua=((x4-x3)*(y1-y3))-((y4-y3)*(x1-x3));
		ub=((x2-x1)*(y1-y3))-((y2-y1)*(x1-x3));
		ua=ua/den;
		ub=ub/den;
		double x = x1+ua*(x2-x1);
		double y = y1+ua*(y2-y1);
		succses = TRUE;
		ta = ua; tb = ub;
		xx = x; yy = y;
		}
}

void winded(CvPoint*p1, CvPoint*p2, CvPoint*p3, CvPoint*p4)
{
	CvPoint avg = {0.25*(p1[0].x+p2[0].x+p3[0].x+p4[0].x), 0.25*(p1[0].y+p2[0].y+p3[0].y+p4[0].y)};
	double ps[4][3] = {atan2f(p1[0].y-avg.y, p1[0].x-avg.x), p1[0].x, p1[0].y,    
					   atan2f(p2[0].y-avg.y, p2[0].x-avg.x), p2[0].x, p2[0].y,														
					   atan2f(p3[0].y-avg.y, p3[0].x-avg.x), p3[0].x, p3[0].y,															
				       atan2f(p4[0].y-avg.y, p4[0].x-avg.x), p4[0].x, p4[0].y };

	double maks,maks1,maks2,maks3;
	double c;
	int i,j;


	for(i=0; i<4; i++)
	{
		maks=ps[i][0];
		maks1=ps[i][1];
		maks2=ps[i][2];
		for(j=i+1; j<4; j++)
		{
			if(ps[j][0]>maks)
			{
				maks= ps[j][0];
				c = ps[i][0];
				ps[i][0]= ps[j][0];
				ps[j][0]= c;

				maks1= ps[j][1];
				c = ps[i][1];
				ps[i][1]= ps[j][1];
				ps[j][1]= c;

				maks2= ps[j][2];
				c = ps[i][2];
				ps[i][2]= ps[j][2];
				ps[j][2]= c;
			}
		}
		ps[i][0]=maks;
		ps[i][1]=maks1;
		ps[i][2]=maks2;
	}
	p1[0].x= ps[0][1];
	p1[0].y= ps[0][2];
	p2[0].x= ps[1][1];
	p2[0].y= ps[1][2];
	p3[0].x= ps[2][1];
	p3[0].y= ps[2][2];
	p4[0].x= ps[3][1];
	p4[0].y= ps[3][2];
}

double ptdst (CvPoint*p1, CvPoint*p2)
{
	return sqrtf(((p1[0].x-p2[0].x)*(p1[0].x-p2[0].x))+((p1[0].y-p2[0].y)*(p1[0].y-p2[0].y)));
}

double compfaces (CvPoint *f1, CvPoint *f2)
{
	double totd = 0;
	int m,n;
	double d,mind;
	for ( m=0; m < 4; m++ )
	{
		mind = 10000;
		for ( n = 0; n < 4; n++ )
		{
			CvPoint*ff1 = &f1[m];
			CvPoint*ff2 = &f2[n];
			d=ptdst(ff1,ff2);
			if (d<mind)
				mind = d;
		}
		totd += mind;
	}
	return totd/4;
}

bool isClose (CvPoint*t1, CvPoint*t2, int t)
{
	return ((abs(t1[0].x-t2[0].x)<t) && (abs(t1[0].y-t2[0].y)<t));
}

void avg (CvPoint*p1, CvPoint*p2,double &avgx, double &avgy)
{
	avgx = (0.5*p1[0].x+0.5*p2[0].x);
	avgy = (0.5*p2[0].y+0.5*p2[0].y);
}

int rgbDistance(int a[], int center[]){
    int temp = 0;
    for(int i=0; i<2; i++){
        temp += abs(center[i] - a[i]);
    }
    return temp;
}

int hsDistance(int a[], int center[]){
    int temp = 0;
    for(int i=0; i<2; i++){
        temp += abs(center[i] - a[i]);
    }
    return temp;
}

/*                                                 END OF PROCEDURE                                              */

int hval[9];
int sval[9];
int vval[9];
int rval[9];
int gval[9];
int bval[9];
int rval2[6][9];
int gval2[6][9];
int bval2[6][9];
int hval2[6][9];
int sval2[6][9];
int vval2[6][9];

int allc[6][9];

int stage = 1;
boolean extract = false;
boolean didassignments = false;

bool pressed = false;
bool det = false;
bool clear = false;
bool procs = false;
char huesatval[] = "";
int colour[6][2][3] = {
		{{255,255,255},{0,0,255}},
		{{0,255,255},{80,255,255}},
		{{0,0,255},{120,255,255}},
		{{0,170,255},{100,255,255}},
		{{255,0,0},{12,220,220}},
		{{0,255,0},{32,220,180}}
	};

int _tmain(int argc, _TCHAR* argv[])
{
	// initialize Data
	subOptLev=-1;
	symRed=1;

	int pos=-1;
	int selected=-1;
	int c,den,W,H,lastdetected,THR,dects,minch;
	int i,j,l,t,k,k1,k2,k3,k4,dd,dd1,dd2,matched,ok1,ok2,evidence,totallines;
	int ep,iii,jjj;
	CvScalar cs[9];
	int hsvcs[9][2];
	double avgx,avgy,ua,ub,x,y,ch,ds1,ds2,ds3,ds4;
	double ii,jj,a,a1,a2,ang,ang1,ang2,pi,qwe,rad;
	bool solved = false;
	bool loaded = false;
	bool solve = false;
	bool success;
	int first = 0;
	char cubestring[68] = "";
	char strings[] = "";

	int centers[6][3];
	int centergb[6][3];

	const char* filename = "cube2h.jpg";

	CvCapture *capture = cvCreateCameraCapture(0);

	IplImage* frame;
	cvNamedWindow( "Rubik's cube tracker", 0 );
	cvResizeWindow("Rubik's cube tracker",640,480);
	frame = cvQueryFrame(capture);
	//frame = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
	CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq* li = 0;

	CvSize sz = cvSize( frame->width & -2, frame->height & -2 );
	den = 2;
	IplImage* sg = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 3 );
	IplImage* sg2 = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 3 );
	IplImage* sg3 = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 3 );
	IplImage* sg4 = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 3 );
	IplImage* sg5 = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 3 );
	IplImage* sgc = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 3 );
	IplImage* hsv = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 3 );
	IplImage* rgb = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 3 );
	IplImage* dst = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 1 );
	IplImage* dst2 = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 1 );
	IplImage* d = cvCreateImage( cvSize(sz.width/den, sz.height/den), IPL_DEPTH_16S, 1 );
	IplImage* d2 = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 1 );
	IplImage* d3 = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 1 );
	IplImage* b = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 1 );
	IplImage* b4 = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 1 );
	IplImage* both = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 1 );
	IplImage* harr = cvCreateImage( cvSize(sz.width/den, sz.height/den), 32, 1 );
	W = sz.width/den;
	H = sz.height/den;
	lastdetected = 0;
	THR  = 100;
	dects = 50; //ideal numbers of line detection
	pi = 3.1415926535897931;

	IplImage* hue = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 1 );
	IplImage* sat = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 1 );
	IplImage* val = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 1 );

	IplImage* red = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 1 );
	IplImage* green = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 1 );
	IplImage* blue = cvCreateImage( cvSize(sz.width/den, sz.height/den), 8, 1 );

	int prevface[3][2] = {{0,0},{5,0},{0,5}};

	bool dodetection = TRUE;

	/* Initialize for LK tracker */
	int succ,flags,tracking,win_size,detected;
	IplImage *image = 0, *grey = 0, *prev_grey = 0, *pyramid = 0, *prev_pyramid = 0, *swap_temp;

	const int MAX_COUNT = 5;
	CvPoint2D32f* points[2] = {0,0}, *swap_points;
	char* status = 0;
	int count = 0;
	int add_remove_pt = 0;

	succ = 0;
	tracking = 0;
	win_size = 5;
	flags = 0;
	detected = 0;

	grey = cvCreateImage( cvSize(W, H), 8, 1 );
	prev_grey = cvCreateImage( cvSize(W, H), 8, 1 );
	pyramid = cvCreateImage( cvSize(W, H), 8, 1 );
	prev_pyramid = cvCreateImage( cvSize(W, H), 8, 1 );

	points[0] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
    points[1] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
	status = (char*)cvAlloc(MAX_COUNT);

	count = 0;

	int counter = 0;
	int undetectednum = 100;
	int add_point = 0;	

	/* initialize font and add text */
    CvFont font,sol;
    cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0, 1, CV_AA);
	cvInitFont(&sol, CV_FONT_HERSHEY_PLAIN, 0.8, 0.8, 0, 1, CV_AA);
	char hueval[50], satval[50], valval[50];
	char redval[50], greenval[50], blueval[50];

	//MEMORY
	//save data to memory pointer
	struct temp {
		CvPoint s1, s2, s3;
		double jarak;
	};
	temp* IT = NULL;
	IT = new temp;

	struct totry {
		CvPoint b1[2000], b2[2000], b3[2000];
		double range[2000];
	};
	totry* PI = NULL;
	PI = new totry;

	struct sudut {
		double angs[2000];
	};
	sudut* angel = NULL;
	angel = new sudut;

	struct res {
		int evi[1000];
		CvPoint b4[1000], b5[1000], b6[1000];
	};
	res* EV = NULL;
	EV = new res;

	struct minps {
		CvPoint b7, b8, b9;
	};
	minps* MI = NULL;
	MI = new minps;

	struct point {
		CvPoint g1[1000], g2[1000], g3[1000], g4[1000];
	};
	point* PT = NULL;
	PT = new point;

	struct lastpoint {
		CvPoint g5[1000], g6[1000], g7[1000];
	};
	lastpoint* LPT = NULL;
	LPT = new lastpoint;

	struct point2 {
		CvPoint p0[1000], v1[1000], v2[1000];
		CvPoint features[1000];
	};
	point2* TT = NULL;
	TT = new point2;

	struct point3 {
		CvPoint ep[1000];
	};
	point3* EP = NULL;
	EP = new point3;

////////////////////////////////////////////////////////////////////////

	while(1) {
	frame = cvQueryFrame(capture);
	//frame = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
	cvResize(frame,sg);
	cvCopy(sg,sgc);
	cvCvtColor(sg,grey,CV_RGB2GRAY);
	
	//tracking mode
	if (tracking>0)	{
		detected = 2;
		//tracking = 0;
		
		CvPoint2D32f ftr [] = {(*TT).features[0].x,(*TT).features[0].y, (*TT).features[1].x,(*TT).features[1].y,
							   (*TT).features[2].x,(*TT).features[2].y, (*TT).features[3].x,(*TT).features[3].y,};

		cvCalcOpticalFlowPyrLK( prev_grey, grey, prev_pyramid, pyramid,
                points[0], points[1], count, cvSize(win_size,win_size), 3, status, 0,
                cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), CV_LKFLOW_PYR_A_READY );
		
		for( i = k4 = 0; i < count; i++ ) {
            if( !status[i] ) continue;
            points[1][k4++] = points[1][i];
			(*TT).features[i].x = points [1][i].x;
			(*TT).features[i].y = points [1][i].y;
        }
        count = k4;	

		if (add_point == 1) {
			points[1][0] = ftr[0];
			points[1][1] = ftr[1];
			points[1][2] = ftr[2];
			points[1][3] = ftr[3];
			(*TT).features[0].x = points [1][0].x;		(*TT).features[0].y = points [1][0].y;
			(*TT).features[1].x = points [1][1].x;		(*TT).features[1].y = points [1][1].y;
			(*TT).features[2].x = points [1][2].x;		(*TT).features[2].y = points [1][2].y;
			(*TT).features[3].x = points [1][3].x;		(*TT).features[3].y = points [1][3].y;
			add_point=0;
			count=4;
		}

		if (count < 4) {
			tracking = 0; //we lost it, restart search again
		}
		else {
			//yakinkan bahwa jarak sudah konsisten
			CvPoint* ft0 = &(*TT).features[0];
			CvPoint* ft1 = &(*TT).features[1];
			CvPoint* ft2 = &(*TT).features[2];
			CvPoint* ft3 = &(*TT).features[3];
			ds1 = ptdst(ft0,ft1);
			ds2 = ptdst(ft2,ft3);
			if (max(ds1,ds2)/min(ds1,ds2) > 1.4) tracking = 0;

			ds3 = ptdst(ft0,ft2);
			ds4 = ptdst(ft1,ft3);
			if (max(ds3,ds4)/min(ds3,ds4) > 1.4) tracking = 0;
			if (ds1< 10 || ds2<10 || ds3<10 || ds4<10) tracking=0;
            if (tracking==0) detected=0; 
		}
	}


	//detection mode
	if(tracking==0){
		detected = 0;
        cvSmooth(grey,dst2,CV_GAUSSIAN, 3);
        cvLaplace(dst2,d);
        cvCmpS(d,8,d2,CV_CMP_GT);

		cvCmpS(grey,100,b,CV_CMP_LT);
        cvAnd(b,d2,d2);

		//threshold adaptif
		if (lastdetected>dects) THR= THR+1;
		if (lastdetected<dects) THR= max(2,(THR-1));
		li = cvHoughLines2( d2, storage, CV_HOUGH_PROBABILISTIC, 1, 3.1415926/45, THR, 10, 5 );

		//tes derajat antar garis
		for( i = 0; i <  li->total; i++ )
		{
			CvPoint* p = (CvPoint*)cvGetSeqElem(li,i);
			//cvLine( sgc, p[0], p[1], CV_RGB(0,255,0), 1, CV_AA,0); 
			a = atan2f(p[1].y-p[0].y,p[1].x-p[0].x);
			if (a<0) a += pi;
			(*angel).angs[i] = a;
		}

		//cek garis yang mempunyai titik akhir sama
		t = 10;
		k= 0;
		for( i = 0; i <  li->total; i++ ) {
			CvPoint* p = (CvPoint*)cvGetSeqElem(li,i);
			for(j = i+1; j <li->total;j++) {
				CvPoint* q = (CvPoint*)cvGetSeqElem(li,j);

				//uji konsistensi panjang garis
				dd1 = sqrtf((p[1].x-p[0].x)*(p[1].x-p[0].x)+(p[1].y-p[0].y)*(p[1].y-p[0].y));
				dd2 = sqrtf((q[1].x-q[0].x)*(q[1].x-q[0].x)+(q[1].y-q[0].y)*(q[1].y-q[0].y));
				if (max(dd1,dd2)/min(dd1,dd2)>1.3) continue;
				
				CvPoint* p1 = &p[0];	CvPoint* p2 = &p[1];
				CvPoint* q1 = &q[0];	CvPoint* q2 = &q[1];

				matched = 0;

				if (isClose(p1,q2,t)) {
					avg(p1,q2,avgx,avgy);
					(*IT).s1.x = avgx;		(*IT).s1.y = avgy;
					(*IT).s2.x = p2[0].x;	(*IT).s2.y = p2[0].y;
					(*IT).s3.x = q1[0].x;	(*IT).s3.y = q1[0].y;
					(*IT).jarak = dd1;
					matched = matched +1;
				}
				if (isClose(p2,q2,t)) {
					avg(p2,q2,avgx,avgy);
					(*IT).s1.x = avgx;		(*IT).s1.y = avgy;
					(*IT).s2.x = p1[0].x;	(*IT).s2.y = p1[0].y;
					(*IT).s3.x = q1[0].x;	(*IT).s3.y = q1[0].y;
					(*IT).jarak = dd1;
					matched = matched +1;
				}
				if (isClose(p1,q1,t)) {
					avg(p1,q1,avgx,avgy);
					(*IT).s1.x = avgx;		(*IT).s1.y = avgy;
					(*IT).s2.x = p2[0].x;	(*IT).s2.y = p2[0].y;
					(*IT).s3.x = q2[0].x;	(*IT).s3.y = q2[0].y;
					(*IT).jarak = dd1;
					matched = matched +1;
				}
				if (isClose(p2,q1,t)) {
					avg(p2,q1,avgx,avgy);
					(*IT).s1.x = avgx;		(*IT).s1.y = avgy;
					(*IT).s2.x = q2[0].x;	(*IT).s2.y = q2[0].y;
					(*IT).s3.x = p1[0].x;	(*IT).s3.y = p1[0].y;
					(*IT).jarak = dd1;
					matched = matched +1;
				}
				
				if (matched == 0) {
					interseg(p1, p2, q1, q2,success,ua,ub,x,y);
					if (success && ua>0 && ua<1 && ub>0 && ub<1) {
						//cvLine( sgc, p[0], p[1], CV_RGB(255,0,0), 1, CV_AA,0);
						ok1 = 0;
						ok2 = 0;

						if (abs(ua-1.0/3)<0.05) ok1=1;	if (abs(ua-2.0/3)<0.05) ok1=2;
						if (abs(ub-1.0/3)<0.05) ok2=1;	if (abs(ub-2.0/3)<0.05) ok2=2;

						//garis bagian dalam cube
						if (ok1>0 && ok2>0){
							//cvLine( sgc, p[0], p[1], CV_RGB(0,255,0), 1, CV_AA,0);
							if (ok1==2) {
								p1 = &p[1];
								p2 = &p[0];
							}
							if (ok2==2) {
								q1 = &q[1];
								q2 = &q[0];
							}
							CvPoint z1[] ={(q1[0].x+2.0/3*(p2[0].x-p1[0].x)),(q1[0].y+2.0/3*(p2[0].y-p1[0].y))};
							CvPoint z2[] ={(p1[0].x+2.0/3*(q2[0].x-q1[0].x)),(p1[0].y+2.0/3*(q2[0].y-q1[0].y))};
							CvPoint z[] ={(p1[0].x-1.0/3*(q2[0].x-q1[0].x)),(p1[0].y-1.0/3*(q2[0].y-q1[0].y))};
							(*IT).s1.x = z[0].x;		(*IT).s1.y = z[0].y;
							(*IT).s2.x = z1[0].x;		(*IT).s2.y = z1[0].y;
							(*IT).s3.x = z2[0].x;		(*IT).s3.y = z2[0].y;
							(*IT).jarak = dd1;
							matched = 1;
						}
					}
				}

				if (matched == 1) {
					//cvLine( sgc, p1[0], p2[0], CV_RGB(0,255,0), 1, CV_AA,0);
					//cvLine( sgc, q1[0], q2[0], CV_RGB(0,255,0), 1, CV_AA,0);
					
					//also test angle
					a1 = atan2f((p2[0].y-p1[0].y),(p2[0].x-p1[0].x));
					a2 = atan2f((q2[0].y-q1[0].y),(q2[0].x-q1[0].x));
					if (a1<0) a1 += pi;
					if (a2<0) a2 += pi;
					ang = abs(abs(a2-a1)-pi/2);
					if (ang < 0.5) {	
						(*PI).b1[k].x = (*IT).s1.x;
						(*PI).b1[k].y = (*IT).s1.y;
						(*PI).b2[k].x = (*IT).s2.x;
						(*PI).b2[k].y = (*IT).s2.y;
						(*PI).b3[k].x = (*IT).s3.x;
						(*PI).b3[k].y = (*IT).s3.y;
						(*PI).range[k] = (*IT).jarak;
						k=k+1;
						//cvCircle(sgc,(*PI).b1[k],5, cvScalar(0,255,255),3);
						//cvLine( sgc, p1[0], p2[0], CV_RGB(0,255,0), 2, CV_AA,0);
					}
				}
			}
		}
		
		k1 = 0;
		for( i = 0; i <  k; i++ ) {
			CvPoint p[] =  {(*PI).b1[i].x,(*PI).b1[i].y};
			CvPoint p1[] = {(*PI).b2[i].x,(*PI).b2[i].y};
			CvPoint p2[] = {(*PI).b3[i].x,(*PI).b3[i].y};
			dd = (*PI).range[i];
			a1 = atan2f(p1[0].y-p[0].y,p1[0].x-p[0].x);
			a2 = atan2f(p2[0].y-p[0].y,p2[0].x-p[0].x);
			if (a1<0) a1 += pi;
			if (a2<0) a2 += pi;
			dd = 1.7*dd;
            evidence=0;
            totallines=0;
			//cvLine( sgc, p[0], p2[0], CV_RGB(0,255,0), 1, CV_AA,0);
			//cvLine( sgc, p[0], p1[0], CV_RGB(255,0,0), 1, CV_AA,0);

			//affine transform to local coords
			CvMat A;
			CvMat v;

			double Am[3][3]= {(p2[0].x-p[0].x),(p1[0].x-p[0].x),p[0].x,(p2[0].y-p[0].y),(p1[0].y-p[0].y),p[0].y,0,0,1 };
			double Ainvm[3][3];
			cvInitMatHeader( &A, 3, 3, CV_64FC1, Am);
			CvMat Ainv = cvMat(3, 3, CV_64FC1, Ainvm);
			cvInv(&A, &Ainv, CV_LU);

			//cek setiap titik kordinat dengan matrix
			for(j = 0; j <li->total;j++) {
				a = (*angel).angs[j];
				ang1 = abs(abs(a-a1)-pi/2);
				ang2 = abs(abs(a-a2)-pi/2);

				if ((ang1 > 0.1) && (ang2 > 0.1)) continue;
                CvPoint* q = (CvPoint*)cvGetSeqElem(li,j);
				qwe = 0.06;
				
				//test one point
				double vm[3][1]= {q[0].x,q[0].y,1};
				double vpm[3][1];
                cvInitMatHeader( &v, 3, 1, CV_64FC1, vm);
				CvMat vp = cvMat(3, 1, CV_64FC1, vpm);
				cvMatMul(&Ainv,&v,&vp);

				if ((vpm[0][0] > 1.1 ) || (vpm[0][0] < -0.1)) continue;
				if ((vpm[1][0] > 1.1 ) || (vpm[1][0] < -0.1)) continue;
				if ((abs(vpm[0][0]-1/3.0)>qwe) && (abs(vpm[0][0]-2/3.0)>qwe) && 
					(abs(vpm[1][0]-1/3.0)>qwe) && (abs(vpm[1][0]-2/3.0)>qwe)) continue;

				//end point
				double vm2[3][1]= {q[1].x,q[1].y,1};
				cvInitMatHeader( &v, 3, 1, CV_64FC1, vm2);
				cvMatMul(&Ainv,&v,&vp);

				if ((vpm[0][0] > 1.1 ) || (vpm[0][0] < -0.1)) continue;
				if ((vpm[1][0] > 1.1 ) || (vpm[1][0] < -0.1)) continue;
				if ((abs(vpm[0][0]-1/3.0)>qwe) && (abs(vpm[0][0]-2/3.0)>qwe) && 
					(abs(vpm[1][0]-1/3.0)>qwe) && (abs(vpm[1][0]-2/3.0)>qwe)) continue;
			    
				//cvCircle(sg,q[0],2, cvScalar(255,255,0),3);
				//cvCircle(sg,q[1],2, cvScalar(255,255,0),3);
				//cvLine( sgc, q[0], q[1], CV_RGB(0,255,255), 1, CV_AA,0);
				evidence += 1;
			}
		//print evindence(tanda/petunjuk)
		(*EV).evi[k1] = evidence;
		(*EV).b4[k1].x = p[0].x;	(*EV).b4[k1].y = p[0].y;
		(*EV).b5[k1].x = p1[0].x;	(*EV).b5[k1].y = p1[0].y;
		(*EV).b6[k1].x = p2[0].x;	(*EV).b6[k1].y = p2[0].y;
		k1 = k1 + 1;
		}

		minch = 10000;
		if (k1>0)
		{	
			k2 = 0;
			//temukan titik yang paling cocok
			for( l = 0; l <  k1; l++ )
			{
				if ((*EV).evi[l]>0.05*dects)
				{
					CvPoint p[]  = {(*EV).b4[l].x,(*EV).b4[l].y};
					CvPoint p1[] = {(*EV).b5[l].x,(*EV).b5[l].y};
					CvPoint p2[] = {(*EV).b6[l].x,(*EV).b6[l].y};
					CvPoint p3[] = {(p2[0].x+p1[0].x-p[0].x),(p2[0].y+p1[0].y-p[0].y)};

					//cvLine( sgc, p[0], p1[0], CV_RGB(0,255,0), 1, CV_AA,0);
					//cvLine( sgc, p[0], p2[0], CV_RGB(255,0,0), 1, CV_AA,0);
					//cvLine( sgc, p2[0], p3[0], CV_RGB(0,255,0), 1, CV_AA,0);
					//cvLine( sgc, p3[0], p1[0], CV_RGB(0,255,0), 1, CV_AA,0);
					//CvPoint cen[] = {(0.5*p2[0].x+0.5*p1[0].x),(0.5*p2[0].y+0.5*p1[0].y)};
					//cvCircle(sgc,cen[0],10, cvScalar(255,255,0),5);
					//cvLine( sgc, cvPoint(0 , cen[0].y), cvPoint(320,cen[0].y), CV_RGB(0,255,0), 1, CV_AA,0);
					//cvLine( sgc, cvPoint(cen[0].x,0), cvPoint(cen[0].x,240), CV_RGB(0,255,0), 1, CV_AA,0);

					CvPoint w[]  = {p[0].x,p[0].y, p1[0].x,p1[0].y, p2[0].x,p2[0].y, p3[0].x,p3[0].y};
					p3[0].x = (prevface[2][0]+prevface[1][0]-prevface[0][0]);
					p3[0].y = (prevface[2][1]+prevface[1][1]-prevface[0][1]);
					CvPoint tc[] = {prevface[0][0],prevface[0][1], prevface[1][0],prevface[1][1],
									prevface[2][0],prevface[2][1], p3[0].x,p3[0].y};

					ch = compfaces(w,tc);
					if (ch<minch)
					{
						minch=ch;
						(*MI).b7.x = p[0].x;		(*MI).b7.y = p[0].y;
						(*MI).b8.x = p1[0].x;		(*MI).b8.y = p1[0].y;
						(*MI).b9.x = p2[0].x;		(*MI).b9.y = p2[0].y;
						k2 = 3;
					}
				}
			}   

			if (k2>0)
			{
				prevface [0][0] = (*MI).b7.x;		prevface [0][1] = (*MI).b7.y;
				prevface [1][0] = (*MI).b8.x;		prevface [1][1] = (*MI).b8.y;
				prevface [2][0] = (*MI).b9.x;		prevface [2][1] = (*MI).b9.y;
				//printf("m = %d\n",minch);
				if (minch<10)
				{
					//CvPoint cen[] = {(0.5*(*MI).b9.x+0.5*(*MI).b8.x),(0.5*(*MI).b9.y+0.5*(*MI).b8.y)};
					//cvCircle(sgc, cen[0],20, cvScalar(255,255,0),5);
					//printf("good enough!\n");
					succ+=1;
					(*PT).g1[0].x = prevface [0][0];	(*PT).g1[0].y = prevface [0][1];
					(*PT).g2[0].x = prevface [1][0];	(*PT).g2[0].y = prevface [1][1];
					(*PT).g3[0].x = prevface [2][0];	(*PT).g3[0].y = prevface [2][1];
					detected = 1;
				}
			}
			else {
				succ = 0;
			}
				
			if (succ>2 && 1)
			{
				k3 = 0;
				for( i=0 ; i<2; i++)
				{
					for( j=0 ; j<2; j++)
					{
						if (i == 0) ii =1.0/3;
						if (i == 1) ii =2.0/3;
						if (j == 0) jj =1.0/3;
						if (j == 1) jj =2.0/3;
						if (k3 ==0) {
							(*PT).g1[0].x = (*TT).p0[0].x+ ii*(*TT).v1[0].x+ jj*(*TT).v2[0].x;
							(*PT).g1[0].y = (*TT).p0[0].y+ ii*(*TT).v1[0].y+ jj*(*TT).v2[0].y;
						}
						if (k3 ==1) {
							(*PT).g2[0].x = (*TT).p0[0].x+ ii*(*TT).v1[0].x+ jj*(*TT).v2[0].x;
							(*PT).g2[0].y = (*TT).p0[0].y+ ii*(*TT).v1[0].y+ jj*(*TT).v2[0].y;
						}
						if (k3 ==2) {
							(*PT).g3[0].x = (*TT).p0[0].x+ ii*(*TT).v1[0].x+ jj*(*TT).v2[0].x;
							(*PT).g3[0].y = (*TT).p0[0].y+ ii*(*TT).v1[0].y+ jj*(*TT).v2[0].y;
						}
						if (k3 ==3) {
							(*PT).g4[0].x = (*TT).p0[0].x+ ii*(*TT).v1[0].x+ jj*(*TT).v2[0].x;
							(*PT).g4[0].y = (*TT).p0[0].y+ ii*(*TT).v1[0].y+ jj*(*TT).v2[0].y;
						}
						k3= k3 +1;
					}
				}
				(*TT).features[0].x = (*PT).g1[0].x;		(*TT).features[0].y = (*PT).g1[0].y;
				(*TT).features[1].x = (*PT).g2[0].x;		(*TT).features[1].y = (*PT).g2[0].y;
				(*TT).features[2].x = (*PT).g3[0].x;		(*TT).features[2].y = (*PT).g3[0].y;
				(*TT).features[3].x = (*PT).g4[0].x;		(*TT).features[3].y = (*PT).g4[0].y;
				tracking = 1;
				add_point=1;
				succ = 0;
			}	
		}


	}
	else{
	//tracking mode
		CvPoint p[]  = {(*TT).features[0].x,(*TT).features[0].y };
		CvPoint p1[] = {(*TT).features[1].x,(*TT).features[1].y };
		CvPoint p2[] = {(*TT).features[2].x,(*TT).features[2].y };

		(*TT).v1[0].x = p1[0].x - p[0].x;
		(*TT).v1[0].y = p1[0].y - p[0].y;
		(*TT).v2[0].x = p2[0].x - p[0].x;
		(*TT).v2[0].y = p2[0].y - p[0].y;

		(*PT).g1[0].x = p[0].x - (*TT).v1[0].x - (*TT).v2[0].x;
		(*PT).g1[0].y = p[0].y - (*TT).v1[0].y - (*TT).v2[0].y;
		(*PT).g2[0].x = p[0].x + 2*(*TT).v2[0].x - (*TT).v1[0].x;
		(*PT).g2[0].y = p[0].y + 2*(*TT).v2[0].y - (*TT).v1[0].y;
		(*PT).g3[0].x = p[0].x + 2*(*TT).v1[0].x - (*TT).v2[0].x;
		(*PT).g3[0].y = p[0].y + 2*(*TT).v1[0].y - (*TT).v2[0].y;

		prevface[0][0] = (*PT).g1[0].x;
		prevface[0][1] = (*PT).g1[0].y;
		prevface[1][0] = (*PT).g2[0].x;
		prevface[1][1] = (*PT).g2[0].y;
		prevface[2][0] = (*PT).g3[0].x;
		prevface[2][1] = (*PT).g3[0].y;
	}
	
	//printf("detected = %d\n ", detected);
	//printf("tracking = %d\n ", tracking);
	///*
	if ((detected || undetectednum < 1) && dodetection)
	{
		if (detected < 1)
		{
			undetectednum+=1;
			(*PT).g1[0].x = (*LPT).g5[0].x;
			(*PT).g1[0].y = (*LPT).g5[0].y;
			(*PT).g2[0].x = (*LPT).g6[0].x;
			(*PT).g2[0].y = (*LPT).g6[0].y;
			(*PT).g3[0].x = (*LPT).g7[0].x;
			(*PT).g3[0].y = (*LPT).g7[0].y;
		}
		if (detected >= 1)
		{
			undetectednum=0;
			(*LPT).g5[0].x = (*PT).g1[0].x;
			(*LPT).g5[0].y = (*PT).g1[0].y;
			(*LPT).g6[0].x = (*PT).g2[0].x;
			(*LPT).g6[0].y = (*PT).g2[0].y;
			(*LPT).g7[0].x = (*PT).g3[0].x;
			(*LPT).g7[0].y = (*PT).g3[0].y;
		}

		//extract the colors
		//convert to HSV
		cvCvtColor(sgc, hsv, CV_RGB2HSV);
		cvSplit(hsv, hue, sat, val, NULL);
		cvCvtColor(sgc, rgb, CV_RGB2BGR);
		cvSplit(rgb, red, green, blue, NULL);

		//do drawing
		CvPoint p3[] = {((*PT).g3[0].x+(*PT).g2[0].x-(*PT).g1[0].x), ((*PT).g3[0].y+(*PT).g2[0].y-(*PT).g1[0].y)};
		cvLine( sg, (*PT).g1[0], (*PT).g2[0] , CV_RGB(0,255,0), 2, CV_AA,0);
		cvLine( sg, (*PT).g2[0], p3[0]       , CV_RGB(0,255,0), 2, CV_AA,0);
		cvLine( sg, p3[0]      , (*PT).g3[0] , CV_RGB(0,255,0), 2, CV_AA,0);
		cvLine( sg, (*PT).g3[0], (*PT).g1[0] , CV_RGB(0,255,0), 2, CV_AA,0);
		
		//first sort the points so that 0 is BL 1 is UL and 2 is BR
		winded(&(*PT).g1[0], &(*PT).g2[0], &(*PT).g3[0], &p3[0]);

		//find the coordinates	
		(*TT).v1[0].x = (*PT).g2[0].x-(*PT).g1[0].x;
		(*TT).v1[0].y = (*PT).g2[0].y-(*PT).g1[0].y;
		(*TT).v2[0].x = p3[0].x-(*PT).g1[0].x;
		(*TT).v2[0].y = p3[0].y-(*PT).g1[0].y;
		(*TT).p0[0].x = (*PT).g1[0].x;
		(*TT).p0[0].y = (*PT).g1[0].y;

		ep =0;
		iii=1;
		jjj=5;
		for (ep = 0;ep <9;ep++)
		{
			(*EP).ep[ep].x = (*TT).p0[0].x + iii*(*TT).v1[0].x/6.0 + jjj*(*TT).v2[0].x/6.0;
			(*EP).ep[ep].y = (*TT).p0[0].y + iii*(*TT).v1[0].y/6.0 + jjj*(*TT).v2[0].y/6.0;
			iii=iii+2;
			if (iii == 7) {
				iii = 1;
				jjj = jjj - 2;
			}
		}

		rad = ptdst(&(*TT).v1[0],&cvPoint(0.0,0.0))/6.0;
		//cs = 0;
		//hsvcs = 0;
		den = 2;

		int avrad = 12;
		for (i=0;i <9;i++)
		{
			if ( ((*EP).ep[i].x>rad)  && ((*EP).ep[i].x<W-rad) && ((*EP).ep[i].y>rad) && ((*EP).ep[i].y<H-rad) )
			{
				//gets average of colors
				cvSetImageROI(sgc, cvRect((*EP).ep[i].x-5,(*EP).ep[i].y-5,  avrad, avrad));
				IplImage *sgc2 = cvCreateImage(cvGetSize(sgc),sgc->depth,sgc->nChannels);
				cvCopy(sgc,sgc2, NULL);
				cvResetImageROI(sgc);

				cvSetImageROI(hue, cvRect((*EP).ep[i].x-5,(*EP).ep[i].y-5,  avrad, avrad));
				IplImage *hue2 = cvCreateImage(cvGetSize(hue),hue->depth,hue->nChannels);
				cvCopy(hue,hue2, NULL);
				cvResetImageROI(hue);

				cvSetImageROI(sat, cvRect((*EP).ep[i].x-5,(*EP).ep[i].y-5,  avrad, avrad));
				IplImage *sat2 = cvCreateImage(cvGetSize(sat),sat->depth,sat->nChannels);
				cvCopy(sat,sat2, NULL);
				cvResetImageROI(sat);

				cvSetImageROI(val, cvRect((*EP).ep[i].x-5,(*EP).ep[i].y-5,  avrad, avrad));
				IplImage *val2 = cvCreateImage(cvGetSize(val),val->depth,val->nChannels);
				cvCopy(val,val2, NULL);
				cvResetImageROI(val);
				CvScalar col = cvAvg(sgc2);

				//gambar lingkaran dan kotak pembentuk cube
				cvCircle(sg,(*EP).ep[i],rad, col,-1);
				if (i==4)
					cvCircle(sg,(*EP).ep[i],rad, cvScalar(0,0,255),2);
				else
					cvCircle(sg,(*EP).ep[i],rad, cvScalar(255,255,255),2);
				//cvRectangle(sg, cvPoint((*EP).ep[i].x-5,(*EP).ep[i].y-5), cvPoint(((*EP).ep[i].x-5)+avrad,((*EP).ep[i].y-5)+avrad), cvScalar(255,255,0));

				CvScalar hueavg = cvAvg(hue2);
				CvScalar satavg = cvAvg(sat2);
				CvScalar valavg = cvAvg(val2);

				sprintf(hueval,"%d",int(hueavg.val[0]));
				sprintf(satval,"%d",int(satavg.val[0]));
				sprintf(valval,"%d",int(valavg.val[0]));

				cvPutText(sg, hueval , cvPoint((*EP).ep[i].x+100, (*EP).ep[i].y),  &font, cvScalar(255,  255, 255, 0));
				cvPutText(sg, satval , cvPoint((*EP).ep[i].x+100, (*EP).ep[i].y+10),  &font, cvScalar(255,  255, 255, 0));
				//cvPutText(sg, valval , cvPoint((*EP).ep[i].x+100, (*EP).ep[i].y+20),  &font, cvScalar(255,  255, 255, 0));

				hval[i] = atoi(hueval);
				sval[i] = atoi(satval);
				vval[i] = atoi(valval);
			}
			
		}
		
		if(pressed){
			det = true;

			for(int i=0;i<9;i++){
				//gets RGB average
				cvSetImageROI(red, cvRect((*EP).ep[i].x-5,(*EP).ep[i].y-5,  10, 10));
				IplImage *red2 = cvCreateImage(cvGetSize(red),red->depth,red->nChannels);
				cvCopy(red,red2, NULL);
				cvResetImageROI(red);

				cvSetImageROI(green, cvRect((*EP).ep[i].x-5,(*EP).ep[i].y-5,  10, 10));
				IplImage *green2 = cvCreateImage(cvGetSize(green),green->depth,green->nChannels);
				cvCopy(green,green2, NULL);
				cvResetImageROI(green);

				cvSetImageROI(blue, cvRect((*EP).ep[i].x-5,(*EP).ep[i].y-5,  10, 10));
				IplImage *blue2 = cvCreateImage(cvGetSize(blue),blue->depth,blue->nChannels);
				cvCopy(blue,blue2, NULL);
				cvResetImageROI(blue);

				CvScalar redavg = cvAvg(red2);
				CvScalar greenavg = cvAvg(green2);
				CvScalar blueavg = cvAvg(blue2);

				sprintf(redval,"%d",int(redavg.val[0]));
				sprintf(greenval,"%d",int(greenavg.val[0]));
				sprintf(blueval,"%d",int(blueavg.val[0]));

				rval[i] = atoi(redval);
				gval[i] = atoi(greenval);
				bval[i] = atoi(blueval);
			}

			centers[selected][0] = hval[4];
			centers[selected][1] = sval[4];
			centers[selected][2] = vval[4];

			centergb[selected][0] = gval[4];
			centergb[selected][1] = bval[4];
			centergb[selected][2] = rval[4];

			for(int n=0;n<9;n++){
				bval2[selected][n] = bval[n];
				gval2[selected][n] = gval[n];
				rval2[selected][n] = rval[n];
				hval2[selected][n] = hval[n];
				sval2[selected][n] = sval[n];
				vval2[selected][n] = vval[n];
				if(n==4) printf("%d hue=%d, saturation=%d, value=%d",pos,hval[n],sval[n],hval[n]);
				//printf("%d r=%d, g=%d b=%d",selected,rval[n],gval[n],bval[n]);
			}
			printf("\n");

			//store hue, sat, and val data
			/*for(int v=0; v<9; v++){
				first++;
				char hvchar[50];
				char svchar[50];
				char vvchar[50];
				if(first>1) strcat(huesatval, ",");
				sprintf(hvchar,"%d",hval[v]);
				sprintf(svchar,"%d",sval[v]);
				sprintf(vvchar,"%d",vval[v]);
				strcat(huesatval, "[");
				strcat(huesatval, hvchar);
				strcat(huesatval, ",");
				strcat(huesatval, svchar);
				strcat(huesatval, ",");
				strcat(huesatval, vvchar);
				strcat(huesatval, "]");				
			}*/
			
			pressed = false;			
		}
	}
	//*/

	//draw each cube faces
	if(det){
		int xa=10, ya=10;
		int xx=25, yx=25;
		int inc=15;
		for(int i=0;i<=pos;i++,xa+=50,xx+=50){
			int stick=0;
			for(int s=0;s<3;s++,ya+=inc,yx+=inc){
				for(int st=0;st<3;st++,xa+=inc,xx+=inc){
					cvRectangle(sg, cvPoint(xa, ya), cvPoint(xx, yx), cvScalar(bval2[i][stick], gval2[i][stick], rval2[i][stick], 0), CV_FILLED, 8, 0);
					cvRectangle(sg, cvPoint(xa, ya), cvPoint(xx, yx), cvScalar(0, 0, 0, 0), 1, 8, 0);
					stick++;
				}
				xa-=45; xx-=45;
			}
			ya-=45; yx-=45;
		}

		//if(solved){
		//	cvRectangle(sg, cvPoint(150, 210), cvPoint(320, 240), cvScalar(0,0,0,50), -1, CV_AA);
		//	//cvPutText(sg, "Solution:", cvPoint(25,80), &font, cvScalar(255,255,255,0));
		//	cvPutText(sg, moveSolution, cvPoint(160,215), &sol, cvScalar(255,255,255,0));
		//}
	}

	//draw selected face highlight
	int xpa, ypa, xpe, ype;
	if(pos<5){
		xpa=10+((selected+1)*50), ypa=10;
		xpe=55+((selected+1)*50), ype=55;
		cvRectangle(sg, cvPoint(xpa, ypa), cvPoint(xpe, ype), cvScalar(0, 0, 255, 0), 1, 8, 0);
	} else {
		xpa=10+(selected*50), ypa=10;
		xpe=55+(selected*50), ype=55;
		cvRectangle(sg, cvPoint(xpa, ypa), cvPoint(xpe, ype), cvScalar(0, 0, 255, 0), 1, 8, 0);
	}

	if(clear){
		pos=-1;
		selected=-1;
		clear=false;
	}

	if(procs){
		printf("\n");
		//char *cbs[6][9] = {{"UUUUUUUUU"},{"LLLLLLLLL"},{"FFFFFFFFF"},{"RRRRRRRRR"},{"BBBBBBBBB"},{"DDDDDDDDD"}};
		int chtemp[6][3];
		char ph[100];
		char p[68];
		int temp;
		int c;
		int tmp2[3];
		for(int i=0;i<6;i++){
			for(int j=0;j<9;j++){
				if(j==4){
					c=i;
				} else {
					tmp2[0] = hval2[i][j];
					tmp2[1] = sval2[i][j];
					tmp2[2] = vval2[i][j];
                    temp = rgbDistance(tmp2, centers[0]);
                    c = 0;
                    for(int x=1; x<6; x++){
                        if(rgbDistance(tmp2, centers[x]) < temp){
                            temp = rgbDistance(tmp2, centers[x]);
                            c=x;
							//cvPutText(sg, "haha", cvPoint(200, 200), &font, cvScalar(0, 0, 0, 0));
							//sprintf(p,"%d",x);
                        }
                    }
                }
				//sprintf(p,"%d",c);
				//strcat(strings,p);
				allc[i][j] = c;
			}
			//strcat(strings,"\n");
		}
		
		for(int i=0;i<6;i++){
			temp=1000;
			c=0;
			for(int j=0;j<6;j++){
				if(rgbDistance(centers[i],colour[j][1]) < temp){
					temp = rgbDistance(centers[i],colour[j][1]);
					c=j;
				}
			}

			chtemp[i][0]=colour[c][0][0];
			chtemp[i][1]=colour[c][0][1];
			chtemp[i][2]=colour[c][0][2];
		}
		for(int i=0;i<6;i++){
			for(int j=0;j<9;j++){
					rval2[i][j] = chtemp[allc[i][j]][2];
					gval2[i][j] = chtemp[allc[i][j]][1];
					bval2[i][j] = chtemp[allc[i][j]][0];
					//printf("%d ",allc[i][j]);
			}
		}
		subOptLev=-1;
		symRed=1;
		sprintf(p, "%d%d %d%d %d%d %d%d %d%d %d%d %d%d %d%d %d%d %d%d %d%d %d%d %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d",
						allc[0][7],allc[2][1], allc[0][5],allc[3][1], allc[0][1],allc[4][1], allc[0][3],allc[1][1],
						allc[5][1],allc[2][7], allc[5][5],allc[3][7], allc[5][7],allc[4][7], allc[5][3],allc[1][7],
						allc[2][5],allc[3][3], allc[2][3],allc[1][5], allc[4][3],allc[3][5], allc[4][5],allc[1][3],
						allc[0][8],allc[2][2],allc[3][0], allc[0][2],allc[3][2],allc[4][0], allc[0][0],allc[4][2],allc[1][0], allc[0][6],allc[1][2],allc[2][0],
						allc[5][2],allc[3][6],allc[2][8], allc[5][0],allc[2][6],allc[1][8], allc[5][6],allc[1][6],allc[4][8], allc[5][8],allc[4][6],allc[3][8]);
		printf("%s, %d\n",p, strlen(p));
		strcpy(cubestring,"");
		for(int i=0; i<strlen(p); i++){
			switch(p[i]){
			case '0': strcat(cubestring, "U"); break;
			case '1': strcat(cubestring, "L"); break;
			case '2': strcat(cubestring, "F"); break;
			case '3': strcat(cubestring, "R"); break;
			case '4': strcat(cubestring, "B"); break;
			case '5': strcat(cubestring, "D"); break;
			default: strcat(cubestring, " "); break;
			}
		}
		printf("Konfigurasi: %s\n", cubestring);

		solved=true;
		procs=false;
	}

	if(solve && solved){
		printf("\nTwo-Phase solver started.\n", cubestring);
		if(!loaded){
			visitedA = (char *)calloc(NGOAL/8+1,1);//initialized to 0 by default
			visitedB = (char *)calloc(NGOAL/8+1,1);
			for (l=0;l<NTWIST;l++)
			movesCloserToTarget[l] = (short *)calloc(NFLIPSLICE*2,2);

			printf("Initializing");
			initSymCubes();
			initMoveCubes();
			initInvSymIdx();
			initSymIdxMultiply();
			initMoveConjugate();
			initMoveBitsConjugate();
			initGESymmetries();
			initTwistConjugate();
			initRawFLipSliceRep();
			initTwistMove();
			initCorn6PosMove();
			initEdge4PosMove();
			initEdge6PosMove();
			initSymFlipSliceClassMove();
			initMovesCloserToTarget();
			initNextMove();
			printf("\n");
			loaded = true;
		}
		char coba[68] = "B2 D2 F2 D' L2 U' B2 L2 B2 R2 U R2 B2 U' L' B R2 U' F2 R' B' U L";
		//printf("Konfigurasi new: %s\n", cubestring);
		cc_a = stringToCubieCube(cubestring);
		solveOptimal(cc_a);
		strcpy(moveSolution,"");
		printf("Solution computed.\n");
		solve = false;
	}

	lastdetected =  li->total;
	
	CV_SWAP( prev_grey, grey, swap_temp );
    CV_SWAP( prev_pyramid, pyramid, swap_temp );
    CV_SWAP( points[0], points[1], swap_points );

    cvShowImage( "Rubik's cube tracker", sg );
	cvShowImage( "Threshold", d2 );
	cvShowImage( "Hough Lines", sgc );
	counter+=1; //global counter

	//cvShowImage( "Threshold", d2 );
	c = cvWaitKey(30);
	if (c==27) {
		break;
	}

	if(32<=c && c<128){
		if(c=='n'){
			selected--;
			if(pos<5){
				if(selected<-1) selected=4;
			}
			else{
				if(selected<0) selected=5;
			}
			
			//printf("pos: %d, selected: %d\n",pos,selected);
		}
		if(c=='m'){
			selected++;
			if(pos<5){
				if(selected>4) selected=-1;
			}
			else {
				if(selected>5) selected=0;
			}
			//printf("pos: %d, selected: %d\n",pos,selected);
		}
		if(c=='s'){
			cvSaveImage("E:\\Computer Vision\\rubiks\\pic.jpg",sg);
		}
		if(c=='a'){
			clear = true;
			solved = false;
		}
		if(c=='x'){
			procs=true;
		}
		if(c=='q'){
			solve=true;
		}
		if(c==' '){
			//printf("pos: %d, selected: %d\n",pos,selected);
			if(pos<5) {
				if(pos==selected) pos++;
				selected++;
			}
			printf("pos: %d, selected: %d\n",pos,selected);
			pressed = true;
			/*FILE *file;
			file = fopen("E:\\Computer Vision\\rubiks\\points.js","w+");
			fprintf(file, "var pts = [");
			fprintf(file, huesatval);
			fprintf(file, "];");
			fclose(file);*/
		}
	}

	}

	return 0;
}


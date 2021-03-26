//---------------------------------------------------------------------------

#ifndef Unit2H
#define Unit2H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ToolWin.hpp>
#include <Math.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdHTTP.hpp>
#include <Vcl.Buttons.hpp>

#define STRICT
#pragma option -w-hid
#include <windows.h>
#include <algorithm>
using std::min;
using std::max;
#include <gdiplus.h>
#include <gdiplusimagecodec.h>
#include <gdiplusimaging.h>
#pragma comment(lib, "gdiplus.lib")
#pragma option -whid

#define MAX_IDATA     5000000
#define MAX_ICNDLS    1600000

#define TYPE_PRICE_CANDLES      1
#define TYPE_PRICE_CSIMPLE      2
#define TYPE_MACDH              3
#define TYPE_HISTOGRAM_1        4
#define TYPE_HISTOGRAM_2        5
#define TYPE_OSCILLATOR         6
#define TYPE_CANDLES_NEWS       7
#define TYPE_ORDERS_1           8
#define TYPE_MULTI_HISTOGRAM_1  9

#define FLOAT_3                3
#define PRICE_POINTS           4
#define EXTREMUMS_PRICE_POINTS 5

typedef struct ConvLetter {
        char    win1251;
        int     unicode;
} Letter;

static Letter g_letters[] = {
        {0x82, 0x201A}, // SINGLE LOW-9 QUOTATION MARK
        {0x83, 0x0453}, // CYRILLIC SMALL LETTER GJE
        {0x84, 0x201E}, // DOUBLE LOW-9 QUOTATION MARK
        {0x85, 0x2026}, // HORIZONTAL ELLIPSIS
        {0x86, 0x2020}, // DAGGER
        {0x87, 0x2021}, // DOUBLE DAGGER
        {0x88, 0x20AC}, // EURO SIGN
        {0x89, 0x2030}, // PER MILLE SIGN
        {0x8A, 0x0409}, // CYRILLIC CAPITAL LETTER LJE
        {0x8B, 0x2039}, // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
        {0x8C, 0x040A}, // CYRILLIC CAPITAL LETTER NJE
        {0x8D, 0x040C}, // CYRILLIC CAPITAL LETTER KJE
        {0x8E, 0x040B}, // CYRILLIC CAPITAL LETTER TSHE
        {0x8F, 0x040F}, // CYRILLIC CAPITAL LETTER DZHE
        {0x90, 0x0452}, // CYRILLIC SMALL LETTER DJE
        {0x91, 0x2018}, // LEFT SINGLE QUOTATION MARK
        {0x92, 0x2019}, // RIGHT SINGLE QUOTATION MARK
        {0x93, 0x201C}, // LEFT DOUBLE QUOTATION MARK
        {0x94, 0x201D}, // RIGHT DOUBLE QUOTATION MARK
        {0x95, 0x2022}, // BULLET
        {0x96, 0x2013}, // EN DASH
        {0x97, 0x2014}, // EM DASH
        {0x99, 0x2122}, // TRADE MARK SIGN
        {0x9A, 0x0459}, // CYRILLIC SMALL LETTER LJE
        {0x9B, 0x203A}, // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
        {0x9C, 0x045A}, // CYRILLIC SMALL LETTER NJE
        {0x9D, 0x045C}, // CYRILLIC SMALL LETTER KJE
        {0x9E, 0x045B}, // CYRILLIC SMALL LETTER TSHE
        {0x9F, 0x045F}, // CYRILLIC SMALL LETTER DZHE
        {0xA0, 0x00A0}, // NO-BREAK SPACE
        {0xA1, 0x040E}, // CYRILLIC CAPITAL LETTER SHORT U
        {0xA2, 0x045E}, // CYRILLIC SMALL LETTER SHORT U
        {0xA3, 0x0408}, // CYRILLIC CAPITAL LETTER JE
        {0xA4, 0x00A4}, // CURRENCY SIGN
        {0xA5, 0x0490}, // CYRILLIC CAPITAL LETTER GHE WITH UPTURN
        {0xA6, 0x00A6}, // BROKEN BAR
        {0xA7, 0x00A7}, // SECTION SIGN
        {0xA8, 0x0401}, // CYRILLIC CAPITAL LETTER IO
        {0xA9, 0x00A9}, // COPYRIGHT SIGN
        {0xAA, 0x0404}, // CYRILLIC CAPITAL LETTER UKRAINIAN IE
        {0xAB, 0x00AB}, // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
        {0xAC, 0x00AC}, // NOT SIGN
        {0xAD, 0x00AD}, // SOFT HYPHEN
        {0xAE, 0x00AE}, // REGISTERED SIGN
        {0xAF, 0x0407}, // CYRILLIC CAPITAL LETTER YI
        {0xB0, 0x00B0}, // DEGREE SIGN
        {0xB1, 0x00B1}, // PLUS-MINUS SIGN
        {0xB2, 0x0406}, // CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
        {0xB3, 0x0456}, // CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I
        {0xB4, 0x0491}, // CYRILLIC SMALL LETTER GHE WITH UPTURN
        {0xB5, 0x00B5}, // MICRO SIGN
        {0xB6, 0x00B6}, // PILCROW SIGN
        {0xB7, 0x00B7}, // MIDDLE DOT
        {0xB8, 0x0451}, // CYRILLIC SMALL LETTER IO
        {0xB9, 0x2116}, // NUMERO SIGN
        {0xBA, 0x0454}, // CYRILLIC SMALL LETTER UKRAINIAN IE
        {0xBB, 0x00BB}, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
        {0xBC, 0x0458}, // CYRILLIC SMALL LETTER JE
        {0xBD, 0x0405}, // CYRILLIC CAPITAL LETTER DZE
        {0xBE, 0x0455}, // CYRILLIC SMALL LETTER DZE
        {0xBF, 0x0457} // CYRILLIC SMALL LETTER YI
};


int convert_utf8_to_windows1251(const char* utf8, char* windows1251, size_t n)
{
    int i = 0;
    int j = 0;
    for(; i < (int)n && utf8[i] != 0; ++i) 
	{
        char prefix = utf8[i];
        char suffix = utf8[i+1];
        if ((prefix & 0x80) == 0) 
		{
            windows1251[j] = (char)prefix;
            ++j;
        } 
		else if ((~prefix) & 0x20) 
		{
            int first5bit = prefix & 0x1F;
            first5bit <<= 6;
            int sec6bit = suffix & 0x3F;
            int unicode_char = first5bit + sec6bit;

            if ( unicode_char >= 0x410 && unicode_char <= 0x44F ) 
			{
                windows1251[j] = (char)(unicode_char - 0x350);
            } 
			else if (unicode_char >= 0x80 && unicode_char <= 0xFF) 
			{
                windows1251[j] = (char)(unicode_char);
            } 
			else if (unicode_char >= 0x402 && unicode_char <= 0x403) 
			{
                windows1251[j] = (char)(unicode_char - 0x382);
            } 
			else 
			{
                int count = sizeof(g_letters) / sizeof(Letter);
                for (int k = 0; k < count; ++k) 
				{
                    if (unicode_char == g_letters[k].unicode) 
					{
                        windows1251[j] = g_letters[k].win1251;
                        goto NEXT_LETTER;
                    }
                }
                // can't convert this char
                return 0;
            }
            NEXT_LETTER:
                        ++i;
                        ++j;
                } else {
                        // can't convert this chars
                        return 0;
                }
    }
    windows1251[j] = 0;
    return 1;
}

// AVal - массив анализируемых данных, Nvl - длина массива должна быть кратна степени 2.
// FTvl - массив полученных значений, Nft - длина массива должна быть равна Nvl.

const double TwoPi = 6.283185307179586;

void FFT(double *AVal, double *FTvl, int Nvl, int Nft)
{
    int i, j, n, m, Mmax, Istp;
    double Tmpr, Tmpi, Wtmp, Theta;
    double Wpr, Wpi, Wr, Wi;
    double *Tmvl;

    n = Nvl * 2; Tmvl = new double[n];

    for (i = 0; i < n; i+=2) 
	{
       Tmvl[i] = 0;
       Tmvl[i + 1] = AVal[i/2];
    }

    i = 1; j = 1;
    while (i < n) 
	{
		if (j > i) 
		{
		    Tmpr = Tmvl[i]; 
			Tmvl[i] = Tmvl[j]; 
			Tmvl[j] = Tmpr;
		    Tmpr = Tmvl[i + 1]; 
			Tmvl[i + 1] = Tmvl[j + 1]; 
			Tmvl[j + 1] = Tmpr;
		}
		i = i + 2; m = Nvl;
		while ((m >= 2) && (j > m)) 
		{
		    j = j - m; m = m >> 1;
		}
		j = j + m;
    }

    Mmax = 2;
    while (n > Mmax) 
	{
		Theta = -TwoPi / Mmax; 
		Wpi = sin(Theta);
		Wtmp = sin(Theta / 2); 
		Wpr = Wtmp * Wtmp * 2;
		Istp = Mmax * 2; 
		Wr = 1; Wi = 0; m = 1;

		while (m < Mmax) 
		{
		    i = m; m = m + 2; Tmpr = Wr; Tmpi = Wi;
		    Wr = Wr - Tmpr * Wpr - Tmpi * Wpi;
		    Wi = Wi + Tmpr * Wpi - Tmpi * Wpr;

		    while (i < n) 
		    {
				j = i + Mmax;
				Tmpr = Wr * Tmvl[j] - Wi * Tmvl[j - 1];
				Tmpi = Wi * Tmvl[j] + Wr * Tmvl[j - 1];

				Tmvl[j] = Tmvl[i] - Tmpr; 
				Tmvl[j - 1] = Tmvl[i - 1] - Tmpi;
				Tmvl[i] = Tmvl[i] + Tmpr; 
				Tmvl[i - 1] = Tmvl[i - 1] + Tmpi;
				i = i + Istp;
		    }
		}

		Mmax = Istp;
    }

    for (i = 0; i < Nft; i++) 
	{
        j = i * 2; 
		FTvl[i] = 2*sqrt(pow(Tmvl[j], 2) + pow(Tmvl[j + 1], 2)) / Nvl;
    }

  delete []Tmvl;
}



typedef unsigned char unsi;

typedef struct tikdata
{
   float price;
   __int64 time;
   int volume;
} tikdata;

typedef struct candlesdata
{
  float priceopen;
  float priceclose;
  float low;
  float high;
  __int64 timeopen;
  __int64 volume;
  
  int indexs[10];                         // соответствие последних закрытых свечек разных таймфреймов

  void SetIndexs(int m1, int m5, int m10, int m15, int m30, int h1, int h4, int d1)
  {
	 indexs[0] = m1;
	 indexs[1] = m5;
	 indexs[2] = m10;
	 indexs[3] = m15;
	 indexs[4] = m30;
	 indexs[5] = h1;
	 indexs[6] = h4;
	 indexs[7] = d1;
  }

} candlesdata;

typedef struct timedaymonthyear_candles
{
   unsi minute;
   unsi hour;
   unsi dayofweek;
   unsi dayofmonth;
   unsi month;
   short int year;
} timedaymonthyear_candles;

typedef struct chbox
{
   short int x;
   short int y;
   short int wd;
   short int ht;
   bool show;
} chbox;

typedef struct frame_orders
{
   short int x;
   short int y;
   short int wd;
   short int ht;
   int id_candle;
} frame_orders;

typedef struct frames
{
  Graphics::TBitmap* bit;               // Для оптимизации отрисовки
  unsi timeframe;
  char* head;
  char* id;                             // Для поиска фрейма по идентификатору
  int x;
  int y;
  int width;
  int height;
  int weight;
  int needwidth;                        // Для отрисовки
  bool AutoWidth;
  unsi typedata;
  int count_data;
  int count_data2;                      // Для мультигистограммы
  void* data;
  void** multidata;                     // Для мультигистограммы
  int smesh_1;                          // Для мультигистограммы
  int smesh_2;                          // Для мультигистограммы
  char** podpisi;
  char** podpisi2;                      // Для мультигистограммы
  int xscrollpos;
  int xscrollwidth;
  int xscrollsize;
  int xsmallscrollpos;                  //для плавного перемещения, когда данных много
  int xsmallscrollwidth;
  bool visible;
  bool allwidth;
  bool notnolformashtab;   

  struct frame_orders* fo;              // Сделки на экране

  struct chbox show_daysofweek[7];      // Какие дни недели показывать на графике
  struct chbox show_hoursofday[14];     // Какие часы показывать на графике
  struct chbox show_years[100];         // Какие годы показывать на графике. Для окошка со сделками

  void SetHead(AnsiString text)
  {
	  if(text == "") return;
	  head = (char*)malloc(text.Length() + 1);
	  CopyMemory(head, text.c_str(), text.Length());
	  head[text.Length()] = '\0';
  }

  void Explode(AnsiString* Res, int &count, AnsiString text, AnsiString delimiter, int mx)
  {
	  count = 0;
	  while(1)
	  {
		   int pos = text.Pos(delimiter);
		   int pos2 = pos;
		   if(pos <= 0) pos2 = text.Length() + 1;
		   AnsiString phr = Trim(text.SubString(1, pos2 - 1));
		   if(phr != "")
		   {
			  Res[count] = phr;
			  int tlen = text.Length();
			  int stpos = pos2 + delimiter.Length();
			  int nlen = tlen - stpos + 1;
			  text = text.SubString(stpos, nlen);
			  
			  count++;
			  if(count >= mx)
                 break;
		   }
		   if(pos <= 0) break;
	  }
  }

  void SetPodpisi(AnsiString ipodpisi)
  {
	  podpisi = (char**)malloc(sizeof(char*)*100);
	  AnsiString podp[100];
	  int count_podpisi = 0;
	  Explode(podp, count_podpisi, ipodpisi, ",", 100);
	  for(int i = 0; i < count_podpisi; i++)
	  {
		  podpisi[i] = (char*)malloc(podp[i].Length() + 1);
		  CopyMemory(podpisi[i], podp[i].c_str(), podp[i].Length());
		  podpisi[i][podp[i].Length()] = '\0';
	  }
  }

  void SetData(float* dt, int ct_dt, AnsiString head_, AnsiString podpisi_)
  {
	  data = dt;
	  count_data = ct_dt;
	  SetHead(head_);
	  SetPodpisi(podpisi_);
  }

  void SetMultiData(float** dt, int smesh1, int smesh2, int ct_dt_1, int ct_dt_2, AnsiString head_, AnsiString podpisi_, AnsiString podpisi2_)
  {
	  multidata = (void**)dt;
	  count_data  = ct_dt_1;
	  count_data2 = ct_dt_2;
	  SetHead(head_);
	  SetPodpisi(podpisi2_);
	  podpisi2 = podpisi;
	  SetPodpisi(podpisi_);
	  smesh_1 = smesh1;
	  smesh_2 = smesh2;
  }

  void SetId(AnsiString id_)
  {
	  if(id_ == "") return;
	  id = (char*)malloc(id_.Length() + 1);
	  CopyMemory(id, id_.c_str(), id_.Length());
	  id[id_.Length()] = '\0';
  }

   void ShowAllDays()
   {
	   for(int i = 0; i < 7; i++)
          show_daysofweek[i].show = true;
   }

   void ShowAllHours()
   {
	   for(int i = 0; i < 14; i++)
		  show_hoursofday[i].show = true;
   }

   void ShowAllYears()
   {
	   for(int i = 0; i < 100; i++)
		  show_years[i].show = true;
   }

   bool ClickShow(int x, int y)
   {
	   struct chbox* sdw = show_daysofweek;
	   for(int i = 0; i < 7; i++)
	   {
		   if(x >= sdw[i].x && x <= sdw[i].x + sdw[i].wd &&
			  y >= sdw[i].y && y <= sdw[i].y + sdw[i].ht)
		   {
			   sdw[i].show = !sdw[i].show;
			   return true;
		   }
	   }
	   struct chbox* shd = show_hoursofday;
	   for(int i = 0; i < 14; i++)
	   {
		   if(x >= shd[i].x && x <= shd[i].x + shd[i].wd &&
			  y >= shd[i].y && y <= shd[i].y + shd[i].ht)
		   {
			   shd[i].show = !shd[i].show;
			   return true;
		   }
	   }

	   return false;
   }

   bool ClickYearShow(int x, int y)
   {
	   struct chbox* sy = show_years;
	   for(int i = 0; i < 100; i++)
	   {
		   if(x >= sy[i].x && x <= sy[i].x + sy[i].wd &&
			  y >= sy[i].y && y <= sy[i].y + sy[i].ht)
		   {
			   sy[i].show = !sy[i].show;
			   return true;
		   }
	   }

	   return false;
   }

} frames;

typedef struct float3
{
  float a;
  float b;
  float c;
} float3;

typedef struct float_f3
{
  float* a;
  float* b;
  float* c;
} float_f3;

typedef struct klasterdata
{
  float price;
  short int volume_bue;
  short int volume_sell;
  struct klasterdata* next;
} klasterdata;

typedef struct klaster
{
  unsi count;
  int volume_bue;
  int volume_sell;
  struct klasterdata* klasters;
  struct klasterdata* kl_end;

  struct klasterdata* SearchKlasterByPrice(float price)
  {
	 if(!klasters || !count) return NULL;
	 struct klasterdata* kl = klasters;
	 while(1)
	 {
		if(kl->price == price)
		   return kl;
		if(kl->next) kl = kl->next;
	    else break;
	 }
	 return NULL;
  }

  struct klasterdata* AddKlaster(float price)
  {
     struct klasterdata* kl = (struct klasterdata*)malloc(sizeof(struct klasterdata));
	 ZeroMemory(kl, sizeof(struct klasterdata));
	 count++;
	 kl->price = price;
	 if(!klasters) klasters = kl;
	 else
		kl_end->next = kl;
	 kl_end = kl;
	 return kl;
  }

  void AddOrEditKlaster(bool up, float price, short int volume)
  {
	 //ищем, есть ли кластер с такой ценой, если есть добавляем туда, если нет создаем новый кластер
	 struct klasterdata* kd = SearchKlasterByPrice(price);
	 if(!kd) kd = AddKlaster(price);
	 if(kd)
	 {
		 if(up) kd->volume_bue  += volume;
		 else   kd->volume_sell += volume;
	 }
	 if(up) volume_bue  += volume;
	 else   volume_sell += volume;
  }

  void AddOrEditKlaster2(float price, int volume_bue, int volume_sell)
  {
	 //ищем, есть ли кластер с такой ценой, если есть добавляем туда, если нет создаем новый кластер
	 struct klasterdata* kd = SearchKlasterByPrice(price);
	 if(!kd) kd = AddKlaster(price);
	 if(kd)
	 {
		 kd->volume_bue  += volume_bue;
		 kd->volume_sell += volume_sell;
	 }
	 volume_bue  += volume_bue;
	 volume_sell += volume_sell;
  }

} klaster;

//новостные данные
typedef struct news
{
  __int64 timedate;
  char* country;
  unsi volatiln;
  char* textres;
  char* text;

  void SetParams(AnsiString itimedate, AnsiString icountry, unsi ivolat,
				 AnsiString itextres, AnsiString itext)
  {
	  TFormatSettings sett;
	  sett.ShortDateFormat = "yyyy/mm/dd";    sett.DateSeparator = '/';
	  sett.ShortTimeFormat = "hh:mm:ss";      sett.TimeSeparator = ':';

	  TDateTime dt = StrToDateTime(itimedate, sett);
	  timedate = RoundTo((dt.Val - 25569.0)*86400, 0);

	  country = (char*)malloc(icountry.Length() + 1);
	  CopyMemory(country, icountry.c_str(), icountry.Length());
	  country[icountry.Length()] = '\0';

	  textres = (char*)malloc(itextres.Length() + 1);
	  CopyMemory(textres, itextres.c_str(), itextres.Length());
	  textres[itextres.Length()] = '\0';

	  text = (char*)malloc(itext.Length() + 1);
	  CopyMemory(text, itext.c_str(), itext.Length());
	  text[itext.Length()] = '\0';

	  volatiln = ivolat;
  }
} news;

typedef struct impulse
{
  int ind_start;
  int ind_end;
  int ind_enter;     //возможная точка входа
  int ind_zamedl;    //начало замедления
  int ind_closeimp;  //конец импульса по объемам
  bool up;
} impulse;


#ifndef IEXTREMUM
#define IEXTREMUM
typedef struct extremum
{
  int   index;
  float val;
  bool  min;
} extremum;
#endif

Gdiplus::GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR  gdiplusToken;

//---------------------------------------------------------------------------
class TFMain : public TForm
{
__published:	// IDE-managed Components
	TMainMenu *MMenu;
	TMenuItem *N1;
	TMenuItem *N2;
	TMenuItem *N3;
	TMenuItem *N4;
	TMenuItem *N5;
	TMenuItem *N6;
	TMenuItem *N7;
	TMenuItem *N8;
	TMenuItem *N9;
	TScrollBar *ScrollBar1;
	TOpenDialog *OD;
	TProgressBar *Simulation;
	TLabel *label_date;
	TButton *TmFrame1;
	TButton *TmFrame5;
	TButton *TmFrame10;
	TButton *TmFrame15;
	TButton *TmFrame30;
	TButton *TmFrame1h;
	TButton *TmFrame4h;
	TButton *TmFrame1d;
	TMenuItem *MACDH11;
	TMenuItem *N10;
	TMenuItem *N11;
	TButton *Button1;
	TCheckBox *AutoSynhronize;
	TMenuItem *N12;
	TEdit *SearchDateTime;
	TButton *Button2;
	TTimer *st;
	TTrackBar *ZoomCandles;
	TTrackBar *ZoomVolumes;
	TCheckBox *dV;
	TTrackBar *HorKlastHt;
	TCheckBox *Simulator;
	TButton *RightSimulPos;
	TButton *LeftSimulPos;
	TTimer *TimerStrelki;
	TButton *Button3;
	TButton *Button4;
	TButton *Button5;
	TButton *Button6;
	TButton *ButLine;
	TSpeedButton *SelTrendUp;
	TSpeedButton *SelTrendDown;
	TButton *Button8;
	TLabel *Itogo;
	TLabel *Marga;
	TButton *Button7;
	TButton *Button9;
	TLabel *Label1;
	TEdit *StLoss;
	TButton *Button10;
	TButton *ekstrBot1;
	TButton *Button11;
	TCheckBox *CloseByVolume;
	TButton *Button12;
	TEdit *DiffForExtr;
	TRadioGroup *RadioGroup1;
	TPanel *PanelTypePrice;
	TRadioButton *PClose;
	TRadioButton *PHigh_Low;
	TCheckBox *Min_menshe_max;
	TCheckBox *KlasternieObemi;
	TCheckBox *Not10hour;
	TButton *Button13;
	TCheckBox *CloseOrder23_30;
	TCheckBox *HeikenAshi;
	TLabel *Label2;
	TEdit *TakeProfit;
	TMenuItem *N13;
	TMenuItem *N14;
	TCheckBox *DopGraphicPrices;
	TMenuItem *N15;
	TButton *Button14;
	TButton *Button15;
	TButton *Button16;
	TButton *Button17;
	TCheckBox *BeziersGrOrders;
	TCheckBox *CurvesGrOrders;
	TCheckBox *LinesGrOrders;
	TCheckBox *Sma7GrOrders;
	TCheckBox *Sma11GrOrders;
	TCheckBox *Sma24GrOrders;
	TButton *Button18;
	TCheckBox *SpectrBuy;
	TCheckBox *SpectrSell;
	TCheckBox *SpectrPrib;
	TCheckBox *SpectrUbyt;
	TMenuItem *N3151;
	TMenuItem *N315cit1;
	void __fastcall ScrollBar1Change(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall N3Click(TObject *Sender);
	void __fastcall FormPaint(TObject *Sender);
	void __fastcall SelTimeFrame(TObject *Sender);
	void __fastcall FormMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall FormMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
	void __fastcall FormMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall MACDH11Click(TObject *Sender);
	void __fastcall N10Click(TObject *Sender);
	void __fastcall N11Click(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall N12Click(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall stTimer(TObject *Sender);
	void __fastcall ZoomCandlesChange(TObject *Sender);
	void __fastcall ZoomVolumesChange(TObject *Sender);
	void __fastcall dVClick(TObject *Sender);
	void __fastcall HorKlastHtChange(TObject *Sender);
	void __fastcall SimulatorClick(TObject *Sender);
	void __fastcall LeftSimulPosClick(TObject *Sender);
	void __fastcall RightSimulPosClick(TObject *Sender);
	void __fastcall TimerStrelkiTimer(TObject *Sender);
	void __fastcall Button4Click(TObject *Sender);
	void __fastcall Button3Click(TObject *Sender);
	void __fastcall Button6Click(TObject *Sender);
	void __fastcall Button5Click(TObject *Sender);
	void __fastcall ButLineClick(TObject *Sender);
	void __fastcall SelTrendClick(TObject *Sender);
	void __fastcall Button8Click(TObject *Sender);
	void __fastcall Button7Click(TObject *Sender);
	void __fastcall Button9Click(TObject *Sender);
	void __fastcall Button10Click(TObject *Sender);
	void __fastcall ekstrBot1Click(TObject *Sender);
	void __fastcall Button11Click(TObject *Sender);
	void __fastcall Button12Click(TObject *Sender);
	void __fastcall Button13Click(TObject *Sender);
	void __fastcall HeikenAshiClick(TObject *Sender);
	void __fastcall N14Click(TObject *Sender);
	void __fastcall ODSelectionChange(TObject *Sender);
	void __fastcall ODShow(TObject *Sender);
	void __fastcall DopGraphicPricesClick(TObject *Sender);
	void __fastcall FormMouseWheel(TObject *Sender, TShiftState Shift, int WheelDelta, TPoint &MousePos, bool &Handled);
	void __fastcall N15Click(TObject *Sender);
	void __fastcall StLossChange(TObject *Sender);
	void __fastcall FormCl(TObject *Sender, TCloseAction &Action);
	void __fastcall FormClQuery(TObject *Sender, bool &CanClose);
	void __fastcall FormClDestroy(TObject *Sender);
	void __fastcall Button14Click(TObject *Sender);
	void __fastcall Button15Click(TObject *Sender);
	void __fastcall Button16Click(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall Button17Click(TObject *Sender);
	void __fastcall BeziersGrOrdersClick(TObject *Sender);
	void __fastcall Button18Click(TObject *Sender);
	void __fastcall SpectrBuyClick(TObject *Sender);
	void __fastcall N315cit1Click(TObject *Sender);

private:	// User declarations
    Gdiplus::Graphics* graphics;
	Graphics::TBitmap* bit;

    int myProcessorThreads;                                              // Всего потоков всех ядер

	int count_frames;
	struct frames iframes[100];
	struct frames* curframe;

	int    count_idata;
	struct tikdata*  idata;

	AnsiString CurFileName;
	bool movegorscroll;
    bool MoveIntoFramePriceCandles;
	int  lastX, lastY;
	bool pushright;
	bool graphics_orders;
    bool spectr_fft;

	void __fastcall  DrawWind(struct frames frame);
	void __fastcall  DrawPriceCandles(struct frames* frame);
	void __fastcall  DrawPriceCSimple(struct frames frame);
	void __fastcall  DrawMacdh(struct frames frame);
	void __fastcall  DrawHistogram(struct frames frame);
	void __fastcall  DrawMultiHistogram(struct frames frame);
	void __fastcall  DrawOscillator(struct frames frame);
	void __fastcall  DrawCandlesNews(struct frames frame);
	void __fastcall  DrawOrdersWind(struct frames* frame);
	void __fastcall  GetCandlesAllTimeFrames();
	void __fastcall  GetCandlesAllTimeFrames2(struct candlesdata* cndls, int count);
	void __fastcall  IndexesCandlesNews();
	void __fastcall  CalcAll();
    void __fastcall CalcMinHourDayMonthYear();                           // Вычислим минуты часы дни месяцы годы для оптимизации скорости анализа
	void __fastcall  CalcVolumesVolatilnostFramesHistogramms();          // Гистограммы объемов и волатильности
	void __fastcall  SetCalcZoomVolume();                                // Вычислим ZoomVolume для выбранного таймфрейма
	int __fastcall   EvStrToTime(AnsiString date, AnsiString time);
	void __fastcall  OpenCadaData(AnsiString FileName);
	char* __fastcall ReadFileD(AnsiString FileName, int& sz);
	AnsiString __fastcall ReadFile(AnsiString FileName);
	void __fastcall  WriteToFile(AnsiString FileName, char* buf, int sz);
	char* __fastcall GetStrC(char* mass, char* end, char c, AnsiString &res);
	int __fastcall   GetStrC(AnsiString str, int start, char c, AnsiString &res);
	AnsiString __fastcall GetTime(__int64 unixtime);
	AnsiString __fastcall GetDate(__int64 unixtime);
	AnsiString __fastcall FormatDig(AnsiString str);
    void __fastcall GetDayData(__int64 unixtime, int &numhour, int &numdayofweek,
	                                 int &numdaym, int &nummonth, int &numyear);
	int __fastcall GetHour(__int64 unixtime);
	int __fastcall GetNumDayOfWeek(__int64 unixtime);
	int __fastcall GetNumDayOfMonth(__int64 unixtime);
	
    double __fastcall RoundSmart(double zn);                             // Округляем значение, исходя из его велечины
	AnsiString __fastcall FormatDigit(double zn);                        // Форматируем число, чтобы оно было читаемым
	float* __fastcall Malloc(int size);                                  // Выделим память и обнулим её
    int* __fastcall Malloc2(int size);
	float** __fastcall Malloc(int size1, int size2);                     // Выделим память и обнулим её
	int** __fastcall Malloc2(int size1, int size2);

	AnsiString srl;
	bool __fastcall RoundT();
	bool __fastcall styler();
	bool ccss;

	int ZoomCandlesPosLast;
	int ZoomVolumesPosLast;
	int HorKlastHtPosLast;

	int curX, curY;

	
	int SimulatorPos;                //позиция симулятора, номер свечки текущего таймфрейма, пока так
	int TSX, TSY;                    //для таймера нажатия стрелок
	bool simul_navig;
	AnsiString curSelTrend;          //выбранный текущий тренд
	int count_urovn;                 //поставленные уровни
	float urovni[10];
	bool curSetUroven;
	bool curSelPrice;                //выбрали цену на графике
	float curpricemposSimple;        //для линии цена на простом графике цен

	// покупаем, продаем в режиме симуляции, ставим стоп лосс заранее
	AnsiString typeOrder;
	float priceOrder;
	float priceStopLoss;
	float priceTakeProfit;
	float totalSum;
	float curMarga;
	void __fastcall SimCalcSums();
	void __fastcall StartOrder(int id_candle, float price, bool up);
	void __fastcall EndOrder(int id_candle, float price);
	//--------------------------------------------------------------------
	AnsiString __fastcall cd(AnsiString num);

	//выявленные импульсы и боковики
	int count_impulses;
	struct impulse impulses[10000];

	int count_extremums;
	struct extremum extremums[10000];

public:		// User declarations
	__fastcall TFMain(TComponent* Owner);

	unsi CurTimeFrame;
	int CurSearchIndexCandles;                           //при поиске дня и времени
    int CurSelIndexCandles;
    int curSelNumOrder;

	void __fastcall DrawAll();

	struct frames* __fastcall AddFrame(unsi type, AnsiString id = "");
    struct frames* __fastcall SearchOrAddFrame(unsi type, AnsiString id);
	struct frames* __fastcall SearchFrame(unsi type, AnsiString id);
	void __fastcall HideFramesOscillators();

	int    count_candles[10];                            // Кол-во свечек по каждому таймфрейму
	struct candlesdata* candles[10];                     // Все тайм-фреймы свечек
	struct timedaymonthyear_candles* tmdmy_candles[10];  // Все заранее вычислинные для оптимизации скорости анализа: минуты, часы, дни недели, дни месяца, месяцы, годы
	
	struct klaster klasters[10][MAX_ICNDLS];             //кластеры и дельта объемов

	int count_news;                                      //новостные данные
	struct news allnews[10000];
	
	int candles_news[10][MAX_ICNDLS];                    //маcсивы соответствия индексов свечек и новостей по всем тайм-фреймам

	float  macdh[MAX_ICNDLS];

	// Массив номеров свечек для рисовки точек на графике. Только на нужном таймфрейме
	int count_points;
	int points_numcndls[100000];
	TColor points_colors[100000];

	void __fastcall SelCurTimeFrame(int curtf);
	void __fastcall OpenTida(AnsiString FileName);
	void __fastcall OpenCada(AnsiString FileName);
    void __fastcall ClearCandles();
};
//---------------------------------------------------------------------------
extern PACKAGE TFMain *FMain;
//---------------------------------------------------------------------------


#endif

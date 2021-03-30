//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#include <math.hpp>
#pragma hdrstop

// Удалён код
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFMain *FMain;
//---------------------------------------------------------------------------
__fastcall TFMain::TFMain(TComponent* Owner)
	: TForm(Owner)
{
       // Удалён код
}
//---------------------------------------------------------------------------
struct frames* __fastcall TFMain::AddFrame(unsi type, AnsiString id)
{
        ZeroMemory(&(iframes[count_frames]), sizeof(struct frames));
        int ylastvis_h = 0;
        for(int i = 0; i < count_frames; i++)
              if(iframes[i].visible)
	            ylastvis_h = iframes[i].y + iframes[i].height;

        if(count_frames > 0)
        {
               iframes[count_frames].visible = true;
               iframes[count_frames].y       = ylastvis_h;
               iframes[count_frames].height  = 200;
               iframes[count_frames].width   = FMain->ClientWidth;
               iframes[count_frames].xscrollwidth = 100;
               iframes[count_frames].typedata     = type;
               iframes[count_frames].SetId(id);
        }
        count_frames++;   
        return (&(iframes[count_frames - 1]));
}
//---------------------------------------------------------------------------
struct frames* __fastcall TFMain::SearchOrAddFrame(unsi type, AnsiString id)
{
       for(int i = 0; i < count_frames; i++)
       {
	      if(iframes[i].typedata == type && (AnsiString)(iframes[i].id) == id)
	            return  &iframes[i];
       }

       return AddFrame(type, id);
}
//---------------------------------------------------------------------------
struct frames* __fastcall TFMain::SearchFrame(unsi type, AnsiString id)
{
       for(int i = 0; i < count_frames; i++)
       {
	      if(iframes[i].typedata == type && (AnsiString)(iframes[i].id) == id)
	            return  &iframes[i];
       }

       return NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFMain::HideFramesOscillators()
{
       for(int i = 0; i < count_frames; i++)
       {
	     if(iframes[i].typedata == TYPE_OSCILLATOR)
	           iframes[i].visible = false;
       }
}
//---------------------------------------------------------------------------
void __fastcall TFMain::ScrollBar1Change(TObject *Sender)
{
      if(bit->Height < ScrollBar1->Position + FMain->ClientHeight)
      {
	     bit->Height = ScrollBar1->Position + FMain->ClientHeight + 200;
	     DrawAll();
      }
      Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::FormResize(TObject *Sender)
{
       bit->Width  = FMain->ClientWidth;
       bit->Height = FMain->ClientHeight;
       ScrollBar1->Height   = FMain->ClientHeight;
       ScrollBar1->Max      = FMain->ClientHeight*4;
       ScrollBar1->PageSize = FMain->ClientHeight;
       ScrollBar1->Left     = FMain->ClientWidth - ScrollBar1->Width - 0;
       ScrollBar1->BringToFront();
       Simulation->Top      = FMain->ClientHeight - Simulation->Height;
       label_date->Top      = Simulation->Top + 1;
       Simulation->Left     = 140;
       Simulation->Width    = FMain->ClientWidth - ScrollBar1->Width - Simulation->Left;
       label_date->BringToFront();
       DrawAll();
}
//---------------------------------------------------------------------------
int __fastcall TFMain::EvStrToTime(AnsiString date, AnsiString time)
{
       //yymmdd hhmmss
       TFormatSettings sett;
       sett.ShortDateFormat = "yyyy-mm-dd";    
       sett.DateSeparator = '-';
       sett.ShortTimeFormat = "hh:mm:ss";      
       sett.TimeSeparator = ':';

       TDateTime dt = StrToDateTime(date.SubString(1, 4) + "-" + date.SubString(5, 2) + "-" +
   				    date.SubString(7, 2) + " " + time.SubString(1, 2) + ":" +
				    time.SubString(3, 2) + ":" + time.SubString(5, 2), sett);
       AnsiString sdt = dt.DateTimeString();

       return RoundTo((dt.Val - 25569.0)*86400, 0);
}
//---------------------------------------------------------------------------
void __fastcall TFMain::OpenTida(AnsiString FileName)
{
       SmartTest->ClearCalcIndicator();
       // Скроем осцилляторы
       HideFramesOscillators();

       if(idata)
       {
	      ZeroMemory(idata, MAX_IDATA*sizeof(struct tikdata));
	      count_idata = 0;
       }

       AnsiString ext = ExtractFileExt(FileName);
       if(ext == ".tida")
       {
	      CurFileName = FileName;
	      int sz = 0;
	      char* mass = ReadFileD(FileName, sz);
	      int ver = 0;
	      CopyMemory(&ver, mass, sizeof(int));
	      if(ver == 1)
	      {
		     CopyMemory(&count_idata, mass + sizeof(int), sizeof(int));
		     if(count_idata > 0 && count_idata < MAX_IDATA)
		     {
			    if(idata)
			    {
				  free(idata);
                                  idata = NULL;
                            }
			    if(!idata)
			         idata = (struct tikdata*)malloc(sizeof(struct tikdata)*count_idata);
			    ZeroMemory(idata, count_idata*sizeof(struct tikdata));
			    CopyMemory(idata, mass + sizeof(int)*2, sizeof(struct tikdata)*count_idata);
		     }
	      }

	      free(mass);

	      AnsiString date_string1 = "";
	      AnsiString date_string2 = "";
	      if(count_idata > 0)
	      {
		      TDateTime* dt = new TDateTime(1970,1,1,0,0,0,0);    //+3 часа - московское время
		      double dtv1 = dt->Val;
		      dt->Val = dtv1 + (double)idata[0].time / (double)(24*3600);
		      date_string1 = dt->DateString();
		      dt->Val = dtv1 + (double)idata[count_idata - 1].time / (double)(24*3600);
		      date_string2 = dt->DateString();
		      delete dt;
	      }
	      label_date->Caption = date_string1 + " - " + date_string2;

	      //Обнулим сделки
              SmartTest->count_orders = 0;
	      //вычислим свечки по всем таймфреймам
	      GetCandlesAllTimeFrames();
	      //расставим соответствие индексов свечек и новостей
	      IndexesCandlesNews();
	      CalcAll();
	      DrawAll();

	      return;
       }
}
//---------------------------------------------------------------------------
void __fastcall TFMain::OpenCada(AnsiString FileName)
{
        SmartTest->ClearCalcIndicator();
        // Скроем осцилляторы
        HideFramesOscillators();

        if(idata)
        {
	        ZeroMemory(idata, MAX_IDATA*sizeof(struct tikdata));
	        count_idata = 0;
        }

        AnsiString ext = ExtractFileExt(FileName);
        if(ext == ".cada")
        {
	       ZeroMemory(candles, MAX_ICNDLS*sizeof(struct candlesdata));
	       ZeroMemory(count_candles, 10*sizeof(int));

	       CurFileName = FileName;
	       int sz = 0;
	       char* mass = ReadFileD(FileName, sz);
	       int ver = 0;
	       CopyMemory(&ver, mass, sizeof(int));
	       if(ver == 1)
	       {
		     CopyMemory(count_candles, mass + sizeof(int), sizeof(int)*10);

		     char* mass2 = mass + sizeof(int)*11;
		     for(int i = 0; i < 10; i++)
		     {
			    CopyMemory(&candles[i], mass2, sizeof(struct candlesdata)*count_candles[i]);
			    mass2 += sizeof(struct candlesdata)*count_candles[i];
		     }
	       }

	       free(mass);

	       AnsiString date_string1 = "";
	       AnsiString date_string2 = "";
	       if(count_candles[0] > 0)
	       {
		      TDateTime* dt = new TDateTime(1970,1,1,0,0,0,0);    //+3 часа - московское время
		      double dtv1 = dt->Val;
		      dt->Val = dtv1 + (double)candles[0][0].timeopen / (double)(24*3600);
		      date_string1 = dt->DateString();
		      dt->Val = dtv1 + (double)candles[0][count_candles[0] - 1].timeopen / (double)(24*3600);
		      date_string2 = dt->DateString();
		      delete dt;
	       }
	       label_date->Caption = date_string1 + " - " + date_string2;

	       //Обнулим сделки
	       SmartTest->count_orders = 0;
	       //расставим соответствие индексов свечек и новостей
	       IndexesCandlesNews();
	       CalcAll();
	       DrawAll();

	       return;
        }
}
//---------------------------------------------------------------------------
void __fastcall TFMain::N3Click(TObject *Sender)
{
        OD->FilterIndex = 2;
        if(!OD->Execute()) return;

        SmartTest->ClearCalcIndicator();
        // Скроем осцилляторы
        HideFramesOscillators();

        count_idata = 0;

        AnsiString ext = ExtractFileExt(OD->FileName);
        if(ext == ".tida")
        {
	      CurFileName = OD->FileName;
	      int sz = 0;
	      char* mass = ReadFileD(OD->FileName, sz);
	      int ver = 0;
	      CopyMemory(&ver, mass, sizeof(int));
	      if(ver == 1)
	      {
		      CopyMemory(&count_idata, mass + sizeof(int), sizeof(int));
		      if(count_idata > 0 && count_idata < MAX_IDATA)
		      {
			     if(idata)
			     {
				    free(idata);
                                    idata = NULL;
                             }
			     if(!idata)
			          idata = (struct tikdata*)malloc(sizeof(struct tikdata)*count_idata);
			     ZeroMemory(idata, count_idata*sizeof(struct tikdata));

			     CopyMemory(idata, mass + sizeof(int)*2, sizeof(struct tikdata)*count_idata);
		      }
	      }

	      free(mass);

	      AnsiString date_string1 = "";
	      AnsiString date_string2 = "";
	      if(count_idata > 0)
	      {
		      TDateTime* dt = new TDateTime(1970,1,1,0,0,0,0);    //+3 часа - московское время
		      double dtv1 = dt->Val;
		      dt->Val = dtv1 + (double)idata[0].time / (double)(24*3600);
		      date_string1 = dt->DateString();
		      dt->Val = dtv1 + (double)idata[count_idata - 1].time / (double)(24*3600);
		      date_string2 = dt->DateString();
		      delete dt;
	      }
	      label_date->Caption = date_string1 + " - " + date_string2;

	      SmartTest->resvirag_tf = -1;

	      //Обнулим сделки
              SmartTest->count_orders = 0;
	      //вычислим свечки по всем таймфреймам
	      GetCandlesAllTimeFrames();
	      // Вычислим минуты часы дни месяцы годы для оптимизации скорости анализа
	      CalcMinHourDayMonthYear();
	      //расставим соответствие индексов свечек и новостей
	      IndexesCandlesNews();
	      CalcAll();

	      // Вычислим ZoomVolume для выбранного таймфрейма
	      SetCalcZoomVolume();

	      // Гистограммы объемов и волатильности
	      CalcVolumesVolatilnostFramesHistogramms();

	      DrawAll();

	      if(EvanNeiroSet)
	      {
		      AnsiString curdt = GetDate(FMain->candles[0][0].timeopen);
		      AnsiString curtm = GetTime(FMain->candles[0][0].timeopen);
		      EvanNeiroSet->DateTimeTxt->Text = curdt + " " + curtm;
	      }

	      return;
        }

        if(ext == ".cada")
        {
	       OpenCadaData(OD->FileName);
               SmartTest->resvirag_tf = -1;

	       // Запомним последний открытый файл данных, чтобы автоматом его открыть при запуске программы
	       WriteToFile(ExtractFilePath(Application->ExeName) + "runfast.txt", ((AnsiString)OD->FileName).c_str(), OD->FileName.Length());

	       return;
        }

        //<TICKER>,<PER>,<DATE>,<TIME>,<LAST>,<VOL>
        for(int ff = 0; ff < OD->Files->Count; ff++)
        {
	       Simulation->Position = 0;
	       AnsiString FileName = OD->Files->Strings[ff];

	       int sz = 0;
	       char* mass = ReadFileD(FileName, sz);
	       if(!mass) continue;
	       Simulation->Max = sz/100;

	       float cur_min = 0;

	       char* smesh = mass;
	       char* startms;
	       bool savedel2 = false;
	       __int64 last_time = 0;
	       float start_offset = 0;
	       int ch = 0;
	       while(1)
	       {
		      int pos = (smesh - mass)/100;
		      if(Simulation->Position < pos)
			     Simulation->Position = pos;

		      AnsiString str = "";
		      AnsiString tt = "";
		      AnsiString date = "";
		      AnsiString time = "";
		      AnsiString last = "";
		      AnsiString vol  = "";
		      char* t_mass = GetStrC(smesh, mass + sz, '\n', str);
		      smesh = t_mass + 1;

		      int p1 = GetStrC(str, 1, ',', tt);
		      int p2 = GetStrC(str, p1 + 1, ',', tt);
		      int p3 = GetStrC(str, p2 + 1, ',', date);
		      int p4 = GetStrC(str, p3 + 1, ',', time);
		      int p5 = GetStrC(str, p4 + 1, ',', last);
		      int p6 = GetStrC(str, p5 + 1, '\n', vol);

		      double lst = 0; bool trylst = TryStrToFloat(Trim(last), lst);
		      __int64 vl = 0; bool tryvl  = TryStrToInt64(Trim(vol), vl);

		      //если заголовочная строчка, то пропускаем
		      if(!trylst) continue;

		      idata[count_idata].price    = lst;
		      idata[count_idata].volume   = vl;
		      idata[count_idata].time = EvStrToTime(date, time);

		      count_idata++;
		      if(count_idata >= MAX_IDATA - 1)
			    break;
		      if(ch >= 1000)
		      {
			      Application->ProcessMessages();
			      ch = 0;
		      }
		      ch++;

		      if(t_mass[0] == '\0') break;
		      if(smesh >= mass + sz) break;
		      continue;

	       }

	       free(mass);


	       AnsiString date_string1 = "";
	       AnsiString date_string2 = "";
	       if(count_idata > 0)
	       {
		       TDateTime* dt = new TDateTime(1970,1,1,0,0,0,0);    //+3 часа - московское время
		       double dtv1 = dt->Val;
		       dt->Val = dtv1 + (double)idata[0].time / (double)(24*3600);
		       date_string1 = dt->DateString();
		       dt->Val = dtv1 + (double)idata[count_idata - 1].time / (double)(24*3600);
		       date_string2 = dt->DateString();
		       delete dt;
	       }
	       label_date->Caption = date_string1 + " - " + date_string2;

	       SmartTest->resvirag_tf = -1;
       }

       //сохраним в быстром формате
       AnsiString file = OD->FileName + ".tida";  
           FILE* fl = NULL;
       fl = fopen(file.c_str(), "wb");
       if(fl)
       {
	        int ver = 1;
	        fwrite(&ver, sizeof(int), 1, fl);
	        fwrite(&count_idata, sizeof(int), 1, fl);
	        if(count_idata > 0)
	        {
		        fwrite(idata, sizeof(struct tikdata)*count_idata, 1, fl);
	        }
	        fclose(fl);
       }

       //Обнулим сделки
       SmartTest->count_orders = 0;
       //вычислим свечки по всем таймфреймам
       GetCandlesAllTimeFrames();
       // Вычислим минуты часы дни месяцы годы для оптимизации скорости анализа
       CalcMinHourDayMonthYear();
       //расставим соответствие индексов свечек и новостей
       IndexesCandlesNews();
       CalcAll();

       // Вычислим ZoomVolume для выбранного таймфрейма
       SetCalcZoomVolume();

       // Гистограммы объемов и волатильности
       CalcVolumesVolatilnostFramesHistogramms();

       DrawAll();

       if(EvanNeiroSet)
       {
		AnsiString curdt = GetDate(FMain->candles[0][0].timeopen);
		AnsiString curtm = GetTime(FMain->candles[0][0].timeopen);
		EvanNeiroSet->DateTimeTxt->Text = curdt + " " + curtm;
       }
}
//---------------------------------------------------------------------------
void __fastcall TFMain::OpenCadaData(AnsiString FileName)
{
	ZoomVolumes->Position = ZoomVolumes->Min;  

	ClearCandles();
	ZeroMemory(count_candles, 10*sizeof(int));

	CurFileName = FileName; 
	int sz = 0;
	char* mass = ReadFileD(FileName, sz);
	int ver = 0;
	CopyMemory(&ver, mass, sizeof(int));
	if(ver == 1)
	{
		CopyMemory(count_candles, mass + sizeof(int), sizeof(int)*10);

		char* mass2 = mass + sizeof(int)*11 + 0;
		for(int i = 0; i < 10; i++)
		{
			if(!candles[i])
			   continue;
			CopyMemory(candles[i], mass2, sizeof(struct candlesdata)*count_candles[i]);
			mass2 += sizeof(struct candlesdata)*count_candles[i];
		}
	}

	free(mass);

	AnsiString date_string1 = "";
	AnsiString date_string2 = "";
	if(count_candles[0] > 0 && candles[0][0].timeopen > 0 && candles[0][0].timeopen < 10000000000)
	{
		TDateTime* dt = new TDateTime(1970,1,1,0,0,0,0);    //+3 часа - московское время
		double dtv1 = dt->Val;
		dt->Val = dtv1 + (double)candles[0][0].timeopen / (double)(24*3600);
		date_string1 = dt->DateString();
		dt->Val = dtv1 + (double)candles[0][count_candles[0] - 1].timeopen / (double)(24*3600);
		date_string2 = dt->DateString();
		delete dt;
	}
	label_date->Caption = date_string1 + " - " + date_string2;

	//Обнулим сделки
	if(SmartTest)
	       SmartTest->count_orders = 0;
	// Вычислим минуты часы дни месяцы годы для оптимизации скорости анализа
	CalcMinHourDayMonthYear();
	//расставим соответствие индексов свечек и новостей
	IndexesCandlesNews();
	CalcAll();

	// Вычислим ZoomVolume для выбранного таймфрейма
	SetCalcZoomVolume();

	// Гистограммы объемов и волатильности
	try
	{
	       CalcVolumesVolatilnostFramesHistogramms();
	}
        catch(...) {}

	DrawAll();

	if(EvanNeiroSet)
	{
		AnsiString curdt = GetDate(FMain->candles[0][0].timeopen);
		AnsiString curtm = GetTime(FMain->candles[0][0].timeopen);
		EvanNeiroSet->DateTimeTxt->Text = curdt + " " + curtm;
	}
}
//---------------------------------------------------------------------------
void __fastcall TFMain::GetDayData(__int64 unixtime, int &numhour,
				int &numdayofweek, int &numdaym, int &nummonth, int &numyear)
{
	TDateTime* dt = new TDateTime(1970,1,1,0,0,0,0);    //+3 часа - московское время
	dt->Val += (double)unixtime / (double)(24*3600);
	numdayofweek = dt->DayOfWeek();
	if(numdayofweek == 0 || numdayofweek > 6)
	   int aa = 11;
	if(numdayofweek == 1) //воскресенье
	   numdayofweek = 8;
	numdayofweek -= 2;

	AnsiString res = dt->FormatString("hh");
	numhour = StrToInt(res) - 1;

	res = dt->FormatString("dd");
	numdaym = StrToInt(res) - 1;

	res = dt->FormatString("mm");
	nummonth = StrToInt(res) - 1;

	res = dt->FormatString("yyyy");
	numyear = StrToInt(res) - 0;

	delete dt;
}
//---------------------------------------------------------------------------
int __fastcall TFMain::GetHour(__int64 unixtime)
{
	TDateTime* dt = new TDateTime(1970,1,1,0,0,0,0);    //+3 часа - московское время
	dt->Val += (double)unixtime / (double)(24*3600);
	AnsiString res = dt->FormatString("hh");
	delete dt;
	return StrToInt(res);
}
//---------------------------------------------------------------------------
int __fastcall TFMain::GetNumDayOfWeek(__int64 unixtime)
{
        TDateTime* dt = new TDateTime(1970,1,1,0,0,0,0);    //+3 часа - московское время
	dt->Val += (double)unixtime / (double)(24*3600);
	int res = dt->DayOfWeek();
	if(res == 0 || res > 6)
	   int aa = 11;
	if(res == 1) //воскресенье
	   res = 8;
	res--;
	delete dt;
	return (res);
}
//---------------------------------------------------------------------------
int __fastcall TFMain::GetNumDayOfMonth(__int64 unixtime)
{
        TDateTime* dt = new TDateTime(1970,1,1,0,0,0,0);    //+3 часа - московское время
	dt->Val += (double)unixtime / (double)(24*3600);
	AnsiString res = dt->FormatString("dd");
	int numdaym = StrToInt(res);
	delete dt;
	return numdaym;
}
//---------------------------------------------------------------------------
// Выделим память и обнулим её
float* __fastcall TFMain::Malloc(int size)
{
	float* mem  = (float*)malloc(sizeof(float)*size);
	if(mem)
	     ZeroMemory(mem, sizeof(float)*size);
	return mem;
}
//---------------------------------------------------------------------------
// Выделим память и обнулим её
int* __fastcall TFMain::Malloc2(int size)
{
	int* mem  = (int*)malloc(sizeof(int)*size);
	if(mem)
	     ZeroMemory(mem, sizeof(int)*size);
	return mem;
}
//---------------------------------------------------------------------------
// Выделим память и обнулим её
float** __fastcall TFMain::Malloc(int size1, int size2)
{
	float** mem = (float**)malloc(sizeof(float*)*size1);
	if(mem)
		for(int i = 0; i < size1; i++)
		{
			mem[i] = (float*)malloc(sizeof(float)*size2);
			if(mem[i])
			   ZeroMemory(mem[i], sizeof(float)*size2);
		}
	return mem;
}
//---------------------------------------------------------------------------
// Выделим память и обнулим её
int** __fastcall TFMain::Malloc2(int size1, int size2)
{
	int** mem = (int**)malloc(sizeof(int*)*size1);
	if(mem)
		for(int i = 0; i < size1; i++)
		{
			mem[i] = (int*)malloc(sizeof(int)*size2);
			if(mem[i])
			   ZeroMemory(mem[i], sizeof(int)*size2);
		}
	return mem;
}
//---------------------------------------------------------------------------
// Гистограммы объемов и волатильности
void __fastcall TFMain::CalcVolumesVolatilnostFramesHistogramms()
{
	// Объем по таймфреймам, часам, дням недели, дням месяца, месяцам, годам
	float* volume_timeframes = Malloc(8);
	float* volume_hours_day  = Malloc(24);
	float* volume_days_week  = Malloc(7);
	float* volume_days_month = Malloc(31);
	float* volume_month_year = Malloc(12);
	float* volume_year       = Malloc(2100);
	int* ct_volume_days_week = Malloc2(7);
	// Средняя волатильность по таймфреймам
	float* avg_volat_timeframes = Malloc(8);
	// Средняя волатильность по дням месяца
	float* avg_volat_days_month = Malloc(31);
	int* ct_avg_volat_days_month = Malloc2(31);

	// Мультигистограммы
	// Объёмы по часам дня для дней недели
	float** volume_hours_day_ofweek = Malloc(24, 7);
	// Объёмы по часам дня для разных годов
	float** volume_hours_day_years = Malloc(24, 2100);
	// Объёмы по часам дня для разных месяцев года
	float** volume_hours_day_month = Malloc(24, 12);
	// Объёмы по дням недели для разных месяцев года
	float** volume_day_ofweek_month = Malloc(7, 12);
	// Средняя волатильность по часам дня для разных таймфреймов
	float** avg_volat_hours_day_timeframes = Malloc(24, 8);
	int** ct_avg_volat_hours_day_timeframes = Malloc2(24, 8);
	// Средняя волатильность для дней недели для разных таймфреймов
	float** avg_volat_day_ofweek_timeframes = Malloc(7, 8);
	int** ct_avg_volat_day_ofweek_timeframes = Malloc2(7, 8);


	try
	{
		// Ставим минус, нужно для того, чтобы правильно определить первый год для гистограммы
		for(int i = 0; i < 2100; i++)
		   volume_year[i] = -1000000;

		for(int i = 0; i < 8; i++)
		{
			// Вычислим объемы по таймфреймам
			float volumes = 0;
			for(int c = 0; c < count_candles[i]; c++)
			{
				// Если некорректное время или цена, то пропустим
				if(candles[i][c].timeopen > 10000000000 || candles[i][c].timeopen < 0 ||
				   candles[i][c].priceclose > 1000000000 || candles[i][c].priceclose < 0)
				{
				     continue;
				}

				volumes += candles[i][c].volume;

				// Для 4 часового и дневного таймфреймов не считаем для часов дня
				if(i < 6)
				{                       
					try
					{                                    
						int houropen = tmdmy_candles[i][c].hour - 1;
						avg_volat_hours_day_timeframes[houropen][i] += candles[i][c].high - candles[i][c].low;
						ct_avg_volat_hours_day_timeframes[houropen][i] ++;
					}
					catch(...)
					{  }
				}
				int dw = tmdmy_candles[i][c].dayofweek - 1;
				avg_volat_day_ofweek_timeframes[dw][i] += candles[i][c].high - candles[i][c].low;
				ct_avg_volat_day_ofweek_timeframes[dw][i] ++;

				if(i == 7)
				{
				        try
					{
						int dw = tmdmy_candles[i][c].dayofmonth - 1;
						avg_volat_days_month[dw] += candles[i][c].volume;
						ct_avg_volat_days_month[dw]++;
					}
					catch(...)
					{ int aa = 11; }
				}
			}
			if(count_candles[i] > 0)
				volumes /= count_candles[i];

			volume_timeframes[i] = volumes;

			if(i == 7 && count_candles[i] > 0)
			{
			   for(int c = 0; c < 31; c++)
				  if(ct_avg_volat_days_month[c] > 0)
					 avg_volat_days_month[c] /= ct_avg_volat_days_month[c];
			}

			if(i < 6)
			for(int c = 0; c < 24; c++)
			{
				if(ct_avg_volat_hours_day_timeframes[c][i] > 0)
				   avg_volat_hours_day_timeframes[c][i] /= ct_avg_volat_hours_day_timeframes[c][i];
			}

			for(int c = 0; c < 7; c++)
			{
				if(ct_avg_volat_day_ofweek_timeframes[c][i] > 0)
				   avg_volat_day_ofweek_timeframes[c][i] /= ct_avg_volat_day_ofweek_timeframes[c][i];
			}

			// На часовом таймфрейме вычислим все остальное
			if(i == 5)
			{
				for(int c = 0; c < count_candles[i]; c++)
				{
					// Если некорректное время или цена, то пропустим
					if(candles[i][c].timeopen > 10000000000 || candles[i][c].timeopen < 0 ||
				       candles[i][c].priceclose > 1000000000 || candles[i][c].priceclose < 0)
					{
					   int aa = 11;
					   continue;
					}

					int numhour, numdayofweek, numdaym, nummonth, numyear;

					try
					{
						numhour      = tmdmy_candles[i][c].hour - 1;
						numdayofweek = tmdmy_candles[i][c].dayofweek - 1;
						numdaym      = tmdmy_candles[i][c].dayofmonth - 1;
						nummonth     = tmdmy_candles[i][c].month - 1;
						numyear      = tmdmy_candles[i][c].year;
					}
					catch(...)
					{ int aa = 11; }

					float vol = candles[i][c].volume;
					volume_hours_day[numhour]      += vol;
					volume_days_week[numdayofweek] += vol;
					volume_days_month[numdaym]     += vol;///1000.0;
					volume_month_year[nummonth]    += vol;
					volume_year[numyear]           += vol;

					ct_volume_days_week[numdayofweek] ++;

					volume_hours_day_ofweek[numhour][numdayofweek]  += vol;
					volume_hours_day_years[numhour][numyear]        += vol;
					volume_hours_day_month[numhour][nummonth]       += vol;
					volume_day_ofweek_month[numdayofweek][nummonth] += vol;
				}

				for(int ds = 0; ds < 7; ds++)
				{
					if(ct_volume_days_week[ds] > 0)
					   volume_days_week[ds] /= ct_volume_days_week[ds];
                                }
			}
		}
	}
	catch(...)
	{ }

        // Определим года для volume_year
        float* start_volume_year = volume_year;
        int smesh_year       = 0;
        int count_years      = 0;
        AnsiString str_years = "";
        int num_start_years  = -1;
        int num_end_years    = 0;
   
        for(int i = 2000; i < 2100; i++)
        {
	        if(volume_year[i] != -1000000)
	        {
		        if(num_start_years < 0)
		        {
			       num_start_years = i;
			       smesh_year = i;
			       start_volume_year = volume_year + i;
		        }

		        num_end_years = i;
		        count_years++;
                }
        }
        for(int i = num_start_years; i <= num_end_years; i++)
        {
	        str_years += IntToStr(i) + ", ";
        }
        if(str_years.Length() > 1)
	       str_years.SetLength(str_years.Length() - 2);

	// Объёмы
	struct frames* frame1 = SearchOrAddFrame(TYPE_HISTOGRAM_1, "_farme3_");        // Найдем уже созданный фрейм по идентификатору, иначе создадим новый
	struct frames* frame2 = SearchOrAddFrame(TYPE_HISTOGRAM_1, "_farme4_");
	struct frames* frame3 = SearchOrAddFrame(TYPE_HISTOGRAM_1, "_farme5_");
	struct frames* frame5 = SearchOrAddFrame(TYPE_HISTOGRAM_1, "_farme7_");
	struct frames* frame4 = SearchOrAddFrame(TYPE_HISTOGRAM_1, "_farme6_");
	struct frames* frame6 = SearchOrAddFrame(TYPE_HISTOGRAM_1, "_farme8_");
	struct frames* frame7 = SearchOrAddFrame(TYPE_MULTI_HISTOGRAM_1, "_farme9_");
	struct frames* frame8 = SearchOrAddFrame(TYPE_MULTI_HISTOGRAM_1, "_farme10_");
	struct frames* frame9 = SearchOrAddFrame(TYPE_MULTI_HISTOGRAM_1, "_farme11_");
	struct frames* frame10 = SearchOrAddFrame(TYPE_MULTI_HISTOGRAM_1, "_farme12_");

	// Волатильность
	struct frames* frame11 = SearchOrAddFrame(TYPE_MULTI_HISTOGRAM_1, "_farme13_");
	struct frames* frame12 = SearchOrAddFrame(TYPE_MULTI_HISTOGRAM_1, "_farme14_");
	struct frames* frame13 = SearchOrAddFrame(TYPE_HISTOGRAM_1, "_farme15_");


	// Удалён код

	frame1->SetData(volume_timeframes, 8, "Средн. объёмы по таймфр.", "1мн, 5мн, 10мн, 15мн, 30мн, 1чс, 4чс, 1дн");
	frame2->SetData(volume_hours_day + 7, 24 - 7, "Объёмы по часам дня", "08, 09, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24");
	frame3->SetData(volume_days_week, 7, "Ср. объёмы по дням нед.", "пн., вт., ср., чт., пт., сб., вс");
	frame4->SetData(volume_days_month, 31, "Объёмы по дням месяца", "01, 02, 03, 04, 05, 06, 07, 08, 09, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31");
	frame6->SetData(volume_month_year, 12, "Объёмы по месяцам в году", "янв., фев., март, апр., май, июнь, июль, авг., сен., окт., ноя., дек.");
	frame5->SetData(start_volume_year, count_years, "Объёмы по годам", str_years);

	frame7->SetMultiData(volume_hours_day_ofweek, 7, 0, 24 - 7, 7, "Объёмы по часам дня для дней недели",
							   "08, 09, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24",
							   "пн., вт., ср., чт., пт., сб., вс");

	frame8->SetMultiData(volume_hours_day_years, 7, smesh_year, 24 - 7, count_years, "Объёмы по часам дня для разных годов",
							   "08, 09, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24",
							   str_years);

	frame9->SetMultiData(volume_hours_day_month, 7, 0, 24 - 7, 12, "Объёмы по часам дня для разных месяцев года",
							   "08, 09, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24",
							   "янв., фев., март, апр., май, июнь, июль, авг., сен., окт., ноя., дек.");

	frame10->SetMultiData(volume_day_ofweek_month, 0, 0, 7, 12, "Объёмы по дням недели для разных месяцев года",
							   "пн., вт., ср., чт., пт., сб., вс",
							   "янв., фев., март, апр., май, июнь, июль, авг., сен., окт., ноя., дек.");

	frame11->SetMultiData(avg_volat_hours_day_timeframes, 7, 0, 24 - 7, 6, "Средняя волатильность по часам дня для разных таймфреймов",
							   "08, 09, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24",
							   "1мн, 5мн, 10мн, 15мн, 30мн, 1чс, 4чс, 1дн");

	frame12->SetMultiData(avg_volat_day_ofweek_timeframes, 0, 0, 7, 6, "Средняя волатильность для дней недели для разных таймфр.",
							   "пн., вт., ср., чт., пт., сб., вс",
							   "1мн, 5мн, 10мн, 15мн, 30мн, 1чс, 4чс, 1дн");

	frame13->SetData(avg_volat_days_month, 31, "Средняя волатильность по дням месяца", "01, 02, 03, 04, 05, 06, 07, 08, 09, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31");

}
//---------------------------------------------------------------------------

// Удалён код

char* __fastcall TFMain::ReadFileD(AnsiString FileName, int& sz)
{
        sz = 0;
        FILE* fl = fopen(FileName.c_str(), "rb");
        if(!fl) return NULL;
        fseek(fl, 0, SEEK_END);
        sz = ftell(fl);  
        fseek(fl, 0, SEEK_SET);
        if(sz <= 0)
        {
	       fclose(fl);
	       return NULL;
        }

        char* mass = NULL;
        try
        {
	      mass = (char*)malloc(sz + 1);
	      if(mass)
	      {
		     fread(mass, sz, 1, fl);
		     mass[sz] = '\0';
              }
        }
        catch(...)
        {
	      fclose(fl);
	      return NULL;
        }

        fclose(fl);
        return mass;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TFMain::ReadFile(AnsiString FileName)
{
	int sz = 0;
	char* buf = ReadFileD(FileName, sz);
	AnsiString res = (AnsiString)buf;
	free(buf);

	return res;
}
//---------------------------------------------------------------------------
void __fastcall TFMain::WriteToFile(AnsiString FileName, char* buf, int sz)
{
	if(!buf || sz <= 0) return;
	FILE* fl = fopen(FileName.c_str(), "wb");
	if(!fl) return;
	fwrite(buf, sz, 1, fl);
        fclose(fl);
}
//---------------------------------------------------------------------------
char* __fastcall TFMain::GetStrC(char* mass, char* end, char c, AnsiString &res)
{
        res = "";
        if(!mass || !end || mass > end || mass[0] == '\0') return NULL;
        if(mass >= end) return NULL;
        int m = (int)mass;
        try
        {
	        while(mass[0] != c)
	        {
		       if(mass[0] == '\0') break;
		       res += (AnsiString)mass[0];
		       if(mass + 1 >= end) break;
		       mass++;
		       int m = (int)mass;
	    }
        }
        catch(...)
        {
	       int m = (int)mass;
               int e = (int)end;
	       int aa = 1;
        }
        if(res != "") res = Trim(res);
        return mass;
}
//---------------------------------------------------------------------------
int __fastcall TFMain::GetStrC(AnsiString str, int start, char c, AnsiString &res)
{
       res = "";
       if(str == "" || start < 1) return 0;
       int ii = 1;
       for(int i = start; i <= str.Length(); i++)
       {
	       ii = i;
	       if(str[i] == c) break;
	       if(str[i] == '.') str[i] = ',';
	       res += (AnsiString)str[i];
       }
   
       return ii;
}
//---------------------------------------------------------------------------

void __fastcall TFMain::FormPaint(TObject *Sender)
{
        int top = 0 + 60;
        int wd = ClientWidth - ScrollBar1->Width;
        int ht = ClientHeight - top - Simulation->Height;     bit->Width;  bit->Height;
        Canvas->CopyRect(Rect(0, top, wd, ht + top), bit->Canvas,
			 Rect(0, ScrollBar1->Position, wd, ht + ScrollBar1->Position));
}
//---------------------------------------------------------------------------
void __fastcall TFMain::DrawAll()
{
        if(!count_frames) return;

        if(DopGraphicPrices->Checked)
	      iframes[1].visible = true;
        else
	      iframes[1].visible = false;

        int ht = bit->Height;
        for(int i = 0; i < count_frames; i++)
	      if(iframes[i].visible && iframes[i].y + iframes[i].height > ht)
		     ht = iframes[i].y + iframes[i].height;
        bit->Height = ht;
        bit->Canvas->Brush->Color = RGB(230, 230, 230);
        if(!movegorscroll)
              bit->Canvas->FillRect(Rect(0, 0, bit->Width, bit->Height));

        //синхронизируем горизонтальные скроллы с графиком цены
        if(AutoSynhronize->Checked)
        for(int i = 1; i < count_frames; i++)
        {
	      if(iframes[i].typedata != TYPE_HISTOGRAM_1 &&
		 iframes[i].typedata != TYPE_HISTOGRAM_2 &&
		 iframes[i].typedata != TYPE_MULTI_HISTOGRAM_1 &&
		 iframes[i].typedata != TYPE_ORDERS_1 &&
				 iframes[i].typedata != TYPE_PRICE_CSIMPLE)
	      {
		     iframes[i].xscrollpos      = iframes[0].xscrollpos;
	             iframes[i].xsmallscrollpos = iframes[0].xsmallscrollpos;
	      }
        }

        // Установим позиции и длину, если нужно
        int xx = 0, yy = 0;
        int minwd = 300;
        int wdmx = ClientWidth - ScrollBar1->Width;
        int curht = 0;
        for(int i = 0; i < count_frames; i++)
        {
	       struct frames* fr = &iframes[i];
	       if(!fr->visible)
		      continue;

	       if(fr->needwidth > 0)
		      fr->width = fr->needwidth;
	       if(fr->allwidth)
		      fr->width = wdmx;

	       int nextxx = xx + fr->width;
	       int nextyy = yy;
	       if(xx + fr->width > wdmx)
	       {
		       if(wdmx - xx - fr->width < minwd)
		       {
			       xx = 0;
			       if(fr->width > wdmx)
				      fr->width = wdmx;
			       nextxx = fr->width;
			       yy += curht;
			       nextyy += curht;
                               curht = fr->height;
		       }
		       else
		       {
			       fr->width = wdmx - xx;
                               nextxx = 0;
			       nextyy += curht;
                       }
	       }

	       fr->x = xx;
	       fr->y = yy;
	       if(fr->height > curht)
		      curht = fr->height;
	       xx = nextxx;
	       yy = nextyy;
        }
	
        if(yy > 0)
               ScrollBar1->Max = yy + 300;

        for(int i = 0; i < count_frames; i++)
        {
	       // Условия, чтобы во время прокрутки гор скролла этого фрейма отрисовывался только он
               // от подвисонов
	       if(movegorscroll && /* curframe &&*/ &iframes[i] != curframe)
                      continue;

	       if(iframes[i].visible &&
		  (iframes[i].typedata == TYPE_PRICE_CANDLES ||
		   iframes[i].typedata == TYPE_PRICE_CSIMPLE ||
		  iframes[i].typedata == TYPE_HISTOGRAM_1 ||
		  iframes[i].typedata == TYPE_HISTOGRAM_2 ||
		  iframes[i].typedata == TYPE_MULTI_HISTOGRAM_1 ||
		  iframes[i].timeframe == CurTimeFrame ||
		  iframes[i].typedata == TYPE_ORDERS_1) )
	       switch(iframes[i].typedata)
	       {
		       case TYPE_PRICE_CANDLES: DrawPriceCandles(&iframes[i]); break;
		       case TYPE_PRICE_CSIMPLE: DrawPriceCSimple(iframes[i]); break;
		       case TYPE_MACDH:         DrawMacdh(iframes[i]);        break;
		       case TYPE_HISTOGRAM_1:   DrawHistogram(iframes[i]);    break;
		       case TYPE_HISTOGRAM_2:   DrawHistogram(iframes[i]);    break;
		       case TYPE_MULTI_HISTOGRAM_1: DrawMultiHistogram(iframes[i]); break;
		       case TYPE_OSCILLATOR:    DrawOscillator(iframes[i]);   break;
		       case TYPE_CANDLES_NEWS:  DrawCandlesNews(iframes[i]);  break;
		       case TYPE_ORDERS_1:      DrawOrdersWind(&iframes[i]);   break;
	       }
        }
        Canvas->Brush->Color = clBtnFace;

        Invalidate();
}
//---------------------------------------------------------------------------

// Удалён код

void __fastcall TFMain::DrawWind(struct frames frame)
{
        bit->Canvas->Brush->Color = RGB(230, 230, 230);
        bit->Canvas->Pen->Width = 2;
        bit->Canvas->FillRect(Rect(frame.x, frame.y + 20, frame.x + frame.width, frame.y + frame.height));
        bit->Canvas->Brush->Color = RGB(150, 150, 150);
        bit->Canvas->FrameRect(Rect(frame.x, frame.y, frame.x + frame.width, frame.y + frame.height));
        bit->Canvas->Brush->Color = clWhite;
        bit->Canvas->FillRect(Rect(frame.x + 2, frame.y + 2, frame.x + frame.width - 2, frame.y + 20));
        bit->Canvas->Font->Name = "Verdana";
        bit->Canvas->Font->Size = 10;
        bit->Canvas->Font->Style = TFontStyles() << fsBold;
        bit->Canvas->Font->Color = clBlack;
        bit->Canvas->TextOutW(frame.x + 10, frame.y + 2, frame.head);
        //scroll
        bit->Canvas->Brush->Color = RGB(150, 150, 150);
        bit->Canvas->FillRect(Rect(frame.x + 2, frame.y + frame.height - 15, frame.x + frame.width - 2, frame.y + frame.height - 2));
        bit->Canvas->Brush->Color = clWhite;
        bit->Canvas->FillRect(Rect(frame.x + frame.xscrollpos, frame.y + frame.height - 15 + 2,
				   frame.x + frame.xscrollpos + frame.xscrollwidth, frame.y + frame.height - 4));
        //small scroll
        bit->Canvas->Brush->Color = RGB(200, 200, 200);
        bit->Canvas->FillRect(Rect(frame.x + frame.xscrollpos + frame.xsmallscrollpos + 5,
				   frame.y + frame.height - 15 + 5,
				   frame.x + frame.xscrollpos + frame.xsmallscrollpos + 20,
				   frame.y + frame.height - 7));


        bit->Canvas->Font->Size = 8;
        bit->Canvas->Brush->Color = RGB(250, 250, 250);
        bit->Canvas->TextOutW(frame.x + frame.width - 40, frame.y + frame.height - 15, "< >");
}
//---------------------------------------------------------------------------

// Удалён код

//---------------------------------------------------------------------------
void __fastcall TFMain::DrawPriceCandles(struct frames* frame_)
{
        if(!frame_ || !frame_->visible)
	      return;

        struct frames frame = *frame_;

        int hthead     = 20;
        int ht_volume  = 70;

        unsi wcndl = ZoomCandles->Position; //40;
        unsi ww    = ZoomCandles->Position;
        unsi szCandle = ww;

        int widthklasters = 600;

        if(!KlasternieObemi->Checked) widthklasters = 0;

        struct candlesdata* cndls = candles[CurTimeFrame];
        struct timedaymonthyear_candles* tmdmy = tmdmy_candles[CurTimeFrame];

        // Спектр
        if(spectr_fft)
        {
	       if(!SmartTest || SmartTest->count_orders <= 0)
		      return;

	       double massfft[128];
	       double massfft_res[128];
	       double min_fft[128];
	       double max_fft[128];
	       int ct_massfft[128];
	       ZeroMemory(massfft, sizeof(double)*128);
	       ZeroMemory(massfft_res, sizeof(double)*128);
	       ZeroMemory(min_fft, sizeof(double)*128);
	       ZeroMemory(max_fft, sizeof(double)*128);
	       ZeroMemory(ct_massfft, sizeof(int)*128);

	       float min = 1000000;
	       float max = -1000000;

               for(int o = 0; o < SmartTest->count_orders; o++)
	       {
		       // Только покупки
		       if(!SpectrSell->Checked && !SmartTest->orders[o].up)
			      continue;
		       // Только продажи
		       if(!SpectrBuy->Checked && SmartTest->orders[o].up)
			      continue;

		       // Только прибыльные
		       if(!SpectrUbyt->Checked && ((SmartTest->orders[o].up && SmartTest->orders[o].price2 < SmartTest->orders[o].price1) ||
			  (!SmartTest->orders[o].up && SmartTest->orders[o].price2 > SmartTest->orders[o].price1)) )
                               continue;
		       // Только убыточные
		       if(!SpectrPrib->Checked && !((SmartTest->orders[o].up && SmartTest->orders[o].price2 < SmartTest->orders[o].price1) ||
			  (!SmartTest->orders[o].up && SmartTest->orders[o].price2 > SmartTest->orders[o].price1)) )
			      continue;

		       int ind = SmartTest->orders[o].id_candle1 - 1;

		       int dt = tmdmy[ind].dayofweek;
		       if(!frame.show_daysofweek[dt - 1].show)
			       continue;
		       int hr = tmdmy[ind].hour;
		       if(!frame.show_hoursofday[hr - 10].show)
			       continue;

	               ZeroMemory(massfft, sizeof(double)*128);
		       float pr = cndls[ind].priceclose;
		       for(int i = ind; i >= ind - 128 && i >= 0; i--)
		       {
			       float pr2 = cndls[i].priceclose - pr;
			       int indm = i - ind + 128 - 1;
			       if(indm < 0) indm = 0;

			       massfft[indm] += fabs(pr2);
			       ct_massfft[indm] ++;
		       }

	               ZeroMemory(massfft_res, sizeof(double)*128);
		       FFT(massfft, massfft_res, 128,  128);
		       for(int i = 0; i < 128; i++)
		       {
			       if(massfft_res[i] == 0)
				      continue;

			       if(min_fft[i] == 0 || massfft_res[i] < min_fft[i])
				      min_fft[i] = massfft_res[i];

			       if(max_fft[i] == 0 || massfft_res[i] > max_fft[i])
				      max_fft[i] = massfft_res[i];

                           if(massfft_res[i] < min) min = massfft_res[i];
		           if(massfft_res[i] > max) max = massfft_res[i];
		       }
	       }

	       int xx = frame.x + frame.width/2 - 64*5;
	       int yy = frame.y + frame.height/2 + 200;

	       float ky = 0;
	       if(max - min != 0) ky = 100.0/(max - min);

	       bit->Canvas->Pen->Color = clTeal;
	       bit->Canvas->Pen->Width = 4;

	       for(int i = 0; i < 128; i++)
	       {
		       bit->Canvas->MoveTo(xx + i*5, yy - max_fft[i]*ky*4*10);
		       bit->Canvas->LineTo(xx + i*5, yy - min_fft[i]*ky*4*10);
	       }
       }
       else
       {
	       // Посчитаем реальное кол-во свечек с учетом скрытых дней и часов
	       int ct_candles = count_candles[CurTimeFrame];
	       if(!frame.show_daysofweek[0].show || !frame.show_daysofweek[1].show || !frame.show_daysofweek[2].show || !frame.show_daysofweek[3].show ||
		  !frame.show_daysofweek[4].show || !frame.show_daysofweek[5].show || !frame.show_daysofweek[6].show ||
		  !frame.show_hoursofday[0].show || !frame.show_hoursofday[1].show || !frame.show_hoursofday[2].show || !frame.show_hoursofday[3].show ||
		  !frame.show_hoursofday[4].show || !frame.show_hoursofday[5].show || !frame.show_hoursofday[6].show || !frame.show_hoursofday[7].show ||
		  !frame.show_hoursofday[8].show || !frame.show_hoursofday[9].show || !frame.show_hoursofday[10].show || !frame.show_hoursofday[11].show ||
		  !frame.show_hoursofday[12].show || !frame.show_hoursofday[13].show )
	       {
		       int ch = 0;
		       for(int i = 0; i < ct_candles; i++)
		       {
			       int dt = tmdmy[i].dayofweek;
			       if(!frame.show_daysofweek[dt - 1].show)
			               continue;
			       
			       int hr = tmdmy[i].hour;
			       if(!frame.show_hoursofday[hr - 10].show)
				       continue;
			       ch++;
		       }
		       ct_candles = ch;
	       }  
	       int volume;

	       float kf = ZoomCandles->Position; //40;
	       if(ct_candles*wcndl / (frame.width - 100) > kf)
		      kf = ct_candles*wcndl / (frame.width - 100);

	       //коэффициент уменьшения взависимости от таймфрейма, чтобы не выходило за границы
	       int kftmfrm = 1;
	       if(CurTimeFrame == 1) kftmfrm = 5;
	       if(CurTimeFrame == 2) kftmfrm = 10;
	       if(CurTimeFrame == 3) kftmfrm = 15;
	       if(CurTimeFrame == 4) kftmfrm = 30;
	       if(CurTimeFrame == 5) kftmfrm = 60;
	       if(CurTimeFrame == 6) kftmfrm = 240;
	       if(CurTimeFrame == 7) kftmfrm = 1440;

	       int istart = frame.xscrollpos*kf/wcndl + frame.xsmallscrollpos;
	       if(istart > ct_candles - 1)
		      istart = ct_candles - 1;
	       if(istart < 0)
		      istart = 0;
	       int ist = istart;
	       int ct_candles2 = count_candles[CurTimeFrame];
	       if(!frame.show_daysofweek[0].show || !frame.show_daysofweek[1].show || !frame.show_daysofweek[2].show || !frame.show_daysofweek[3].show ||
	          !frame.show_daysofweek[4].show || !frame.show_daysofweek[5].show || !frame.show_daysofweek[6].show ||
	          !frame.show_hoursofday[0].show || !frame.show_hoursofday[1].show || !frame.show_hoursofday[2].show || !frame.show_hoursofday[3].show ||
	          !frame.show_hoursofday[4].show || !frame.show_hoursofday[5].show || !frame.show_hoursofday[6].show || !frame.show_hoursofday[7].show ||
	          !frame.show_hoursofday[8].show || !frame.show_hoursofday[9].show || !frame.show_hoursofday[10].show || !frame.show_hoursofday[11].show ||
	          !frame.show_hoursofday[12].show || !frame.show_hoursofday[13].show )
	       {
		       int ch = 0;
		       for(int i = 0; i < ct_candles2; i++)
		       {
			       int dt = tmdmy[i].dayofweek;
			       if(!frame.show_daysofweek[dt - 1].show)
				       continue;
			       
			       int hr = tmdmy[i].hour;
			       if(!frame.show_hoursofday[hr - 10].show)
				       continue;
			       
			       if(ch == istart)
			       {
				       istart = i;
				       break;
			       }
			       ch++;
		       }
	       }

	       int xsm = istart*wcndl;

	       struct klaster* klast = klasters[CurTimeFrame];

	       if(!cndls)
		      return;

	       int sred   = frame.y + (frame.height - ht_volume - hthead)/2 + hthead;
	       int sredkl = frame.y + frame.height/2 + hthead;

	       int xsmesh = ww/2;
	       if(xsmesh < 5) xsmesh = 5;

	       //кол-во свечек на видимой части графика
	       int kolvo = (frame.width - xsmesh*2 - widthklasters)/wcndl;
	       //кол-во свечек с кластерами
	       int count_klast_candles = 4;

	       __int64 sttm = GetTickCount();

	       if(Simulator->Checked && simul_navig)
	       {
		       //проверяем выходит ли за границу свечка в позиции SimulatorPos
		       //если выходит то передвигаем в конец окошка
		       if(SimulatorPos > istart + kolvo)
		       {
			       istart = SimulatorPos - kolvo;
			       xsm = istart*wcndl;
			       float newpos = ((float)istart)*(float)wcndl/(float)kf;
			       frame.xscrollpos = newpos;
			       newpos -= frame.xscrollpos;
			       float ostatokCndls = newpos*(float)kf/(float)wcndl;
			       frame.xsmallscrollpos = ostatokCndls + 1;
			       iframes[0].xscrollpos      = frame.xscrollpos;
			       iframes[0].xsmallscrollpos = frame.xsmallscrollpos;
		       }
	       }

	       sttm = GetTickCount() - sttm;
	       sttm = GetTickCount();

	       int sdvig = 60;
	       if(sdvig > kolvo)
		      sdvig = kolvo - 5;
	       if(sdvig < 0)
		      sdvig = 0;
	       CurSearchIndexCandles -= sdvig;
	       if(CurSearchIndexCandles < 0)
		      CurSearchIndexCandles = 0;


	       int curCurSearchIndexCandles = CurSearchIndexCandles;
	       if(curCurSearchIndexCandles > 0 && frame.typedata == TYPE_PRICE_CANDLES)
	       {
		       // Посчитаем CurSearchIndexCandles с учетом скрытых дней и часов
		       if(!frame.show_daysofweek[0].show || !frame.show_daysofweek[1].show || !frame.show_daysofweek[2].show || !frame.show_daysofweek[3].show ||
			  !frame.show_daysofweek[4].show || !frame.show_daysofweek[5].show || !frame.show_daysofweek[6].show ||
			  !frame.show_hoursofday[0].show || !frame.show_hoursofday[1].show || !frame.show_hoursofday[2].show || !frame.show_hoursofday[3].show ||
			  !frame.show_hoursofday[4].show || !frame.show_hoursofday[5].show || !frame.show_hoursofday[6].show || !frame.show_hoursofday[7].show ||
			  !frame.show_hoursofday[8].show || !frame.show_hoursofday[9].show || !frame.show_hoursofday[10].show || !frame.show_hoursofday[11].show ||
			  !frame.show_hoursofday[12].show || !frame.show_hoursofday[13].show )
		       {
			       int ch = 0;
			       for(int i = 0; i < ct_candles2; i++)
			       {
				       int dt = tmdmy[i].dayofweek;
				       if(!frame.show_daysofweek[dt - 1].show)
					       continue;
				       
				       int hr = tmdmy[i].hour;
				       if(!frame.show_hoursofday[hr - 10].show)
					       continue;
				       
				       if(i >= curCurSearchIndexCandles)
				       {
					       curCurSearchIndexCandles = ch;
					       break;
				       }
				       ch++;
			       }
		       }

		       frame.xscrollpos = floor(((float)curCurSearchIndexCandles - (float)frame.xsmallscrollpos)*wcndl/(float)kf);
		       iframes[0].xscrollpos = frame.xscrollpos;

		       if(AutoSynhronize->Checked)
		      {
				  for(int i = 0; i < count_frames; i++)
				  {
					  if(iframes[i].typedata != TYPE_HISTOGRAM_1 &&
						 iframes[i].typedata != TYPE_HISTOGRAM_2 &&
						 iframes[i].typedata != TYPE_MULTI_HISTOGRAM_1 &&
						 iframes[i].typedata != TYPE_ORDERS_1 &&
					  iframes[i].typedata != TYPE_PRICE_CSIMPLE)
					  {
						 iframes[i].xscrollpos      = iframes[0].xscrollpos;
						 iframes[i].xsmallscrollpos = iframes[0].xsmallscrollpos;
					  }
				  }
		       }
	   }

	   sttm = GetTickCount() - sttm;
	   sttm = GetTickCount();

	   int maxicandles = count_candles[CurTimeFrame];
	   //если в режиме симулятора, то последняя видимая свечка это позиция симулятора
	   if(Simulator->Checked)
	   {
		   if(maxicandles > SimulatorPos)
			  maxicandles = SimulatorPos;
	   }

	   short int curdayofmonth_draw = 0;
	   short int curmonth_draw      = 0;
	   short int curyear_draw       = 0;
	   short int curhour_draw       = 0;

	   int new_xhourmin = 0, end_xhourmin = 0, lasthourtextdraw = 0, new_xdtmonth = 0, end_xdtmonth = 0;

	   //коэф. масштабирования по высоте
	   float min = 1000000 /*cndls[istart].low*/, max = -1000000 /*cndls[istart].high*/;
	   //макс. объем для отрисовки уровня объемов
	   int maxvol = 0;
	   int autosmesh = 0;
	   for(int i = istart; i < kolvo + istart + autosmesh && i < maxicandles; i++)
	   {
		   // Показываем разрешенные дни недели и часы
		   //int dt = GetNumDayOfWeek(cndls[i].timeopen);
		   int dt = tmdmy[i].dayofweek;
		   if(!frame.show_daysofweek[dt - 1].show)
		   {
			   autosmesh++;
			   continue;
		   }
		   //int hr = GetHour(cndls[i].timeopen);
		   int hr = tmdmy[i].hour;
		   if(!frame.show_hoursofday[hr - 10].show)
		   {
			   autosmesh++;
			   continue;
		   }

		   if(cndls[i].low < min)  min = cndls[i].low;
		   if(cndls[i].high > max) max = cndls[i].high;
		   if(cndls[i].volume > maxvol) maxvol = cndls[i].volume;
	   }

	   // Минимум и максимум для сделок, если показываем графики сделок
	   int CT_CANDLES_ORDERS_CRAPHICS = 200;
	   if(SmartTest && graphics_orders)
	   {

		   min = 1000000;
		   max = -1000000;
		   float up   = 0;
		   float down = 0;
		   for(int o = 0; o < SmartTest->count_orders; o++)
		   {
			   // Только покупки
			   if(!SmartTest->orders[o].up)
				  continue;

			   int ind = SmartTest->orders[o].id_candle1 - 1;

			   int dt = tmdmy[ind].dayofweek;
			   if(!frame.show_daysofweek[dt - 1].show)
				   continue;
			   int hr = tmdmy[ind].hour;
			   if(!frame.show_hoursofday[hr - 10].show)
				   continue;

			   float diffup = 0;
			   float diffdn = 0;
			   float pr = cndls[ind].priceclose;
			   for(int i = ind; i >= ind - CT_CANDLES_ORDERS_CRAPHICS && i >= 0; i--)
			   {
				   float pr2 = cndls[i].priceclose - pr;

				   if(pr2 > 0 && pr2 > diffup)       diffup = pr2;
				   if(pr2 < 0 && fabs(pr2) > diffdn) diffdn = fabs(pr2);
			   }
			   if(diffup > up)   up   = diffup;
			   if(diffdn > down) down = diffdn;
		   }
		   min = -down;
		   max = up;
	   }

	   float ky = 1;
	   if(max - min != 0 && max - min > 0.000000000001)     //высота заголовка, высота скролла, объемы, отступ снизу и сверху
		  ky = ((float)frame.height - hthead - 20 - 20*3 - ht_volume)/(max - min);
	   float avg = (min + max)/2.0;

	   float mv = (float)maxvol/1000.0;
	   mv = Ceil(mv);
	   if(mv <= 0) mv = 1;
	   mv *= 1000;

	   sttm = GetTickCount() - sttm;
	   sttm = GetTickCount();

	   DrawWind(frame);

	   if(graphics)  delete graphics; graphics = NULL;
	   if(!graphics && bit->Canvas->Handle) graphics = new Gdiplus::Graphics(bit->Canvas->Handle);
	   if(!graphics) return;
	   if(graphics) graphics->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	   Gdiplus::Pen gpen(Gdiplus::Color(205, 180, 180, 255), 1);

	   float sred_avg_ky = sred + avg*ky;

	   bit->Canvas->Font->Name = "Tahoma";
	   bit->Canvas->Font->Size = 8;
	   bit->Canvas->Font->Style = TFontStyles();
	   SetBkMode(bit->Canvas->Handle, TRANSPARENT);

	   // Нарисуем сетку
	   float roundmax = RoundTo(max*2.0, 3)/2.0;
	   float diffp = 500.0;
	   if(500.0*ky < 50)
	   {
		   diffp = 1000.0;
		   roundmax = RoundTo(max, 3);
	   }

	   int ct_lines = (roundmax - min)/diffp;
	   int tl1 = frame.x + xsmesh + widthklasters;
	   int tl2 = frame.x + frame.width;
	   for(int l = 0; l <= ct_lines; l++)
	   {
		   float price = roundmax - l*diffp;
		   int y = sred_avg_ky - (price)*ky;
		   if(y < frame.y + hthead + 40)
			  continue;
		   graphics->DrawLine(&gpen, tl1, y, tl2, y);

		   AnsiString str_price = FloatToStr(price);
		   if(price > 999)
			  str_price = str_price.SubString(1, str_price.Length() - 3) + " " + str_price.SubString(str_price.Length() - 3 + 1, 3);

		   if(RoundTo(price, 3) != price)
			  bit->Canvas->Font->Size = 7.4;
		   else
			  bit->Canvas->Font->Size = 8;
		   TSize sz = bit->Canvas->TextExtent(str_price);
		   bit->Canvas->TextOut(tl2 - 5 - sz.cx, y - sz.cy, str_price);
	   }

	   bit->Canvas->Font->Color = RGB(0, 0, 200);   ZoomVolumes->Position;   ZoomVolumes->Max;

	   //уровень объемов
	   float zoomvol = (float)ZoomVolumes->Position*300.0/(float)ZoomVolumes->Max/(float)ZoomVolumes->Max;
	   if(maxicandles > 0)
	   {
		   int t1 = frame.x + xsmesh + widthklasters/* - xsm*/;
		   int t2 = frame.x + xsmesh + widthklasters + (maxicandles - istart)*wcndl/* - xsm*/;
		   if(t2 > frame.x + frame.width - 40)
			  t2 = frame.x + frame.width - 40;
		   int ty = frame.y + frame.height - 20 - maxvol*zoomvol /*mv*ZoomVolumes->Position/500000.0*/;
		   bit->Canvas->Pen->Width = 1;
		   bit->Canvas->Pen->Color = RGB(250, 250, 250);
		   bit->Canvas->MoveTo(t1, ty);
		   bit->Canvas->LineTo(t2, ty);
		   bit->Canvas->TextOutW(t1 + 10, ty - 10, IntToStr((int)mv));
	   }

	   // Рисуем график до сделки на одном месте
	   Gdiplus::Pen pen_green(Gdiplus::Color(128, 0, 200, 0), 1);
	   Gdiplus::Pen pen_red(Gdiplus::Color(128, 200, 0, 0), 1);
	   if(SmartTest && graphics_orders)
	   {
		   __int64 sttime = GetTickCount();

		   bit->Canvas->Pen->Color = RGB(50, 50, 50);
		   float pricest = -1000000;
		   for(int o = 0; o < SmartTest->count_orders; o++)
		   {
			   // Только покупки
			   if(!SmartTest->orders[o].up)
				  continue;

			   int i = SmartTest->orders[o].id_candle1;

			   // Показываем разрешенные дни недели и часы
			   int dt = tmdmy[i].dayofweek;
			   if(!frame.show_daysofweek[dt - 1].show)
			   {
				   continue;
			   }
			   int hr = tmdmy[i].hour;
			   if(!frame.show_hoursofday[hr - 10].show)
			   {
				   continue;
			   }

			   int ind = SmartTest->orders[o].id_candle1 - 1;
			   int st = ind - CT_CANDLES_ORDERS_CRAPHICS;
			   if(st < 0) st = 0;
			   int xx = frame.x + frame.width*0.5 - (ind - st)*wcndl/2;
			   pricest = cndls[ind].priceclose;
			   float pricesmesh = pricest - cndls[ind].priceclose;

			   bit->Canvas->Pen->Color = RGB(0, 200, 0);
			   bool prib = true;
			   if((SmartTest->orders[o].up && SmartTest->orders[o].price2 < SmartTest->orders[o].price1) ||
				  (!SmartTest->orders[o].up && SmartTest->orders[o].price2 > SmartTest->orders[o].price1) )
			   {
				   bit->Canvas->Pen->Color = RGB(200, 0, 0);
				   prib = false;
			   }

			   Gdiplus::PointF points[201];
			   Gdiplus::PointF points2[201];
			   for(int c = st; c <= ind; c++)
			   {
				   struct candlesdata cndl1 = cndls[c];
				   struct candlesdata cndl2 = cndls[c + 1];  

				   int yy1 = sred_avg_ky - (cndl1.priceclose - pricest)*ky/1.0;
				   int yy2 = sred_avg_ky - (cndl2.priceclose - pricest)*ky/1.0;

				   points[c - st].X = xx + wcndl*(c - st);
				   points[c - st].Y = yy1;
				   points2[c - st].X = xx + wcndl*(c - st);
				   points2[c - st].Y = yy1;
			   }
			   if(Sma7GrOrders->Checked || Sma11GrOrders->Checked || Sma24GrOrders->Checked)
			   {
				   int ct_sma = 7;
				   if(Sma11GrOrders->Checked) ct_sma = 11;
				   if(Sma24GrOrders->Checked) ct_sma = 24;

				   for(int c = 0; c < 200; c++)
				   {
					   int ch = 0;
					   float sum = 0;
					   for(int s = c; s > c - ct_sma && s >= 0; s--)
					   {
						   sum += (float)points2[s].Y;
						   ch++;
					   }
					   sum /= (float)ch;
					   points[c].Y = sum;
				   }
			   }

			   Gdiplus::Pen* peno;
			   if(prib) peno = &pen_green;
			   else peno = &pen_red;

			   if(LinesGrOrders->Checked || Sma7GrOrders->Checked || Sma11GrOrders->Checked || Sma24GrOrders->Checked)
				   graphics->DrawLines(peno, points, ind - st + 1);
			   if(BeziersGrOrders->Checked)
				   graphics->DrawBeziers(peno, points, ind - st + 1);
			   if(BeziersGrOrders->Checked)
				   graphics->DrawCurve(peno, points, ind - st + 1);
		   }
		   sttime = GetTickCount() - sttime;
		   sttime = 0;
	   }
	   else
	   {
		   Gdiplus::Pen pen(Gdiplus::Color(255, 180, 180, 180), 1);
		   Gdiplus::Pen pen2(Gdiplus::Color(255, 100, 100, 100), 2);

		   __int64 sttmin1 = 0, sttmin2 = 0, sttmin3 = 0, sttmin4 = 0, sttmin5 = 0, sttmin6 = 0;
		   __int64 sttmin1_1 = 0, sttmin1_2 = 0, sttmin1_3 = 0, sttmin1_4 = 0;

		   //номер свечки по позиции мыши
		   int numcandlempos = -1;

		   int lastposDateTime = 0;
		   AnsiString lastdt = "";

		   // Для пропуска свечек
		   autosmesh = 0;
		   int smesh_empty = 0;
		   int realistart = -1;
		   int realiend   = 0;
		   // Реальные позиции свечек
		   int realx[100000];
		   ZeroMemory(realx, sizeof(int)*100000);

		   int t1 = frame.x + xsmesh + widthklasters + istart*wcndl - xsm - wcndl;
		   for(int i = istart; i <= kolvo + istart + autosmesh && i < maxicandles; i++)
		   {
			   // Показываем разрешенные дни недели и часы
			   int dt = tmdmy[i].dayofweek;
			   if(!frame.show_daysofweek[dt - 1].show)
			   {
				   autosmesh++;
				   smesh_empty = 20;
				   // Чтобы конец рамки сделки рисовался в пропуске между свечками
				   if(realistart >= 0)
					  realx[i - realistart] = t1 + 10;
				   continue;
			   }
			   int hr = tmdmy[i].hour;
			   if(!frame.show_hoursofday[hr - 10].show)
			   {
				   autosmesh++;
				   smesh_empty = 20;
				   // Чтобы конец рамки сделки рисовался в пропуске между свечками
				   if(realistart >= 0)
					  realx[i - realistart] = t1 + 10;
				   continue;
			   }
			   if(realistart < 0)
				  realistart = i;
			   realiend = i;

			  //оптимизируем
			  struct candlesdata cndl = cndls[i];
			  struct candlesdata cndl2;
			  if(i > 0)
				 cndl2 = cndls[i - 1];
			  //int t1 = frame.x + xsmesh + widthklasters + i*wcndl - xsm;
			  t1 += wcndl + smesh_empty;
			  smesh_empty = 0;

			  realx[i - realistart] = t1;

			  //найдем свечку по координатам мыши
			  if(curX > 0 && curY > 0)
			  {
				  if(curX >= t1 - ww/2 && curX <= t1 + ww/2)
				  {
					  numcandlempos = i;
				  }
			  }

			  __int64 sttmin = GetTickCount();
			  __int64 sttmin1_ = GetTickCount();

			  // верт. линии
			  // Если коэффициент маленький(свечки узкие), то не рисуем линии
			  if(ww >= 10 && t1 < frame.x + frame.width - 50)
			  {
				  bit->Canvas->Pen->Width = 1;
				  bit->Canvas->Pen->Color = RGB(210, 210, 210);
				  bit->Canvas->MoveTo(t1, frame.y + 50);
				  bit->Canvas->LineTo(t1, frame.y + frame.height - 30);
			  }

			  sttmin1_ = GetTickCount() - sttmin1_;
			  sttmin1_1 += sttmin1_;
			  sttmin1_ = GetTickCount();

			  //хвосты свечки
			  if(!HeikenAshi->Checked)
			  {
				  bit->Canvas->Pen->Width = 2;
				  if(ww < 10)
					 bit->Canvas->Pen->Width = 1;
				  bit->Canvas->Pen->Color = RGB(100, 100, 100);
				  if(cndl.priceclose > cndl.priceopen)
				  {
					  bit->Canvas->MoveTo(t1, sred_avg_ky - (cndl.high)*ky);
					  bit->Canvas->LineTo(t1, sred_avg_ky - (cndl.priceclose)*ky);
					  bit->Canvas->MoveTo(t1, sred_avg_ky - (cndl.priceopen)*ky);
					  bit->Canvas->LineTo(t1, sred_avg_ky - (cndl.low)*ky);
				  }
				  else
				  {
					  bit->Canvas->MoveTo(t1, sred_avg_ky - (cndl.high)*ky);
					  bit->Canvas->LineTo(t1, sred_avg_ky - (cndl.priceopen)*ky);
					  bit->Canvas->MoveTo(t1, sred_avg_ky - (cndl.priceclose)*ky);
					  bit->Canvas->LineTo(t1, sred_avg_ky - (cndl.low)*ky);
				  }
			  }

			  sttmin1_ = GetTickCount() - sttmin1_;
			  sttmin1_2 += sttmin1_;
			  sttmin1_ = GetTickCount();

			  //объем
			  float zoomvol = (float)ZoomVolumes->Position*300.0/(float)ZoomVolumes->Max/(float)ZoomVolumes->Max;
			  bit->Canvas->Pen->Width = ww/2;
			  if(ww <= 2) bit->Canvas->Pen->Width = ww;
			  {
				  bit->Canvas->Brush->Color = RGB(100, 100, 250);
				  int xx = t1 - Ceil((float)ww/2.0);
				  int margx = 1;
				  if(ww > 3) xx ++;
				  else margx = 0;
				  bit->Canvas->FillRect(Rect(xx, frame.y + frame.height - 20 - cndl.volume*zoomvol,
											 xx + ww - margx, frame.y + frame.height - 20));
			  }

			  sttmin1_ = GetTickCount() - sttmin1_;
			  sttmin1_3 += sttmin1_;
			  sttmin1_ = GetTickCount();

			  //объем покупок
			  if(dV->Checked)
			  {
				 if(klast[i].volume_bue >= klast[i].volume_sell)
				 {
					bit->Canvas->Pen->Color = RGB(100, 250, 100);
					bit->Canvas->MoveTo(t1, frame.y + frame.height - 20 - klast[i].volume_bue*zoomvol);
					bit->Canvas->LineTo(t1, frame.y + frame.height - 20);
					bit->Canvas->Pen->Color = RGB(250, 100, 100);
					bit->Canvas->MoveTo(t1, frame.y + frame.height - 20 - klast[i].volume_sell*zoomvol);
					bit->Canvas->LineTo(t1, frame.y + frame.height - 20);
				 }
				 else
				 {
					bit->Canvas->Pen->Color = RGB(250, 100, 100);
					bit->Canvas->MoveTo(t1, frame.y + frame.height - 20 - klast[i].volume_sell*zoomvol);
					bit->Canvas->LineTo(t1, frame.y + frame.height - 20);
					bit->Canvas->Pen->Color = RGB(100, 250, 100);
					bit->Canvas->MoveTo(t1, frame.y + frame.height - 20 - klast[i].volume_bue*zoomvol);
					bit->Canvas->LineTo(t1, frame.y + frame.height - 20);
				 }
			  }

			  sttmin1_ = GetTickCount() - sttmin1_;
			  sttmin1_4 += sttmin1_;
			  sttmin1_ = GetTickCount();

			  sttmin = GetTickCount() - sttmin;
			  sttmin1 += sttmin;
			  sttmin = GetTickCount();

			  //цвета тел свечек
			  TColor clr_up = RGB(80, 200, 80);
			  TColor clr_dn = RGB(225, 100, 100);


			  if(!HeikenAshi->Checked)
			  {
				  //тело свечки
				  if(cndl.priceopen < cndl.priceclose)
					 bit->Canvas->Brush->Color = clr_up;
				  else bit->Canvas->Brush->Color = clr_dn;

				  // Отступ для свечки слева и справа
				  int margxcndl = 0;
				  if(ww > 10)
					 margxcndl = 2;

				  bit->Canvas->FillRect(Rect(t1 - szCandle/2.0 + margxcndl, RoundTo(sred - (cndl.priceopen - avg)*ky, 0),
							     t1 + szCandle/2.0 - margxcndl, RoundTo(sred - (cndl.priceclose - avg)*ky, 0)));
				  //выделим тело свечки рамкой того же оттенка цвета при наведении
				  if(numcandlempos == i)
				  {
					  if(cndl.priceopen < cndl.priceclose)
						 bit->Canvas->Pen->Color = RGB(220, 255, 220);
					  else bit->Canvas->Pen->Color = RGB(255, 170, 170);

					  bit->Canvas->FrameRect(Rect(t1 - szCandle/2.0, RoundTo(sred - (cndl.priceopen - avg)*ky, 0),
								      t1 + szCandle/2.0, RoundTo(sred - (cndl.priceclose - avg)*ky, 0)));

				  }

				 bit->Canvas->Pen->Width = 1;
				 bit->Canvas->Pen->Color = clWhite;
				 for(int pc = 0; pc < count_points; pc++)
				 {
					 if(points_numcndls[pc] > i)
						break;                        
					 if(points_numcndls[pc] == i)
					 {
						 bit->Canvas->Brush->Color = points_colors[pc];
						 bit->Canvas->Ellipse(t1 - 5, RoundTo(sred - (cndl.priceclose - avg)*ky, 0) - 5, t1 + 5,
									      RoundTo(sred - (cndl.priceclose - avg)*ky, 0) + 5);
					 }
				 }
			  }

			  sttmin = GetTickCount() - sttmin;
			  sttmin2 += sttmin;
			  sttmin = GetTickCount();

			  //HeikenAshi свечки
			  if(HeikenAshi->Checked && i > 0)
			  {
				  float HAopen = (cndl2.priceopen + cndl2.priceclose)/2.0;
				  float HAclose = (cndl.priceopen + cndl.priceclose + cndl.high + cndl.low)/4.0;
				  float HAhigh = (cndl.high + cndl.priceopen + cndl.priceclose)/3.0;
				  float HAlow  = (cndl.low + cndl.priceopen + cndl.priceclose)/3.0;

				  bit->Canvas->Pen->Width = 2;
				  if(ww <= 3) bit->Canvas->Pen->Width = 1;
				  bit->Canvas->Pen->Color = RGB(100, 100, 100);
				  bit->Canvas->MoveTo(t1, sred - (HAhigh - avg)*ky);
				  bit->Canvas->LineTo(t1, sred - (HAlow - avg)*ky);

				  if(HAopen < HAclose)
					 bit->Canvas->Brush->Color = clr_up;
				  else bit->Canvas->Brush->Color = clr_dn;

				  // Отступ для свечки слева и справа
				  int margxcndl = 0;
				  if(ww > 10)
					 margxcndl = 2;
				  bit->Canvas->FillRect(Rect(t1 - szCandle/2.0 + margxcndl, RoundTo(sred - (HAopen - avg)*ky, 0),
							     t1 + szCandle/2.0 - margxcndl, RoundTo(sred - (HAclose - avg)*ky, 0)));
			  }

			  sttmin  = GetTickCount() - sttmin;
			  sttmin3 += sttmin;
			  sttmin  = GetTickCount();


			  // удален код

			  sttmin = GetTickCount() - sttmin;
			  sttmin4 += sttmin;
			  sttmin = GetTickCount();

			  //если режим симуляции и последняя свечка симуляции то рисуем под временем направление тренда,
			  //указанного пользователем
			  if(Simulator->Checked && i == maxicandles - 1)
			  {
				  if(curSelTrend == "up")   bit->Canvas->Font->Color = clGreen;
				  if(curSelTrend == "down") bit->Canvas->Font->Color = clRed;
				  bit->Canvas->Brush->Color = clWhite;
				  bit->Canvas->Font->Size = 10;
				  bit->Canvas->TextOutW(t1 - 30, frame.y + hthead + 40, curSelTrend);
				  bit->Canvas->Font->Color = clBlack;
				  bit->Canvas->Font->Size  = 6;
			  }

			  if(!SmartTest)
				 continue;
				 
			  //рисуем индикаторы, применяемые в тесте
			  struct all_data_instruments* adi = SmartTest->curAllDataInstruments;
			  while(adi)
			  {
				 if(adi->res && adi->GetNumericParam(1) == CurTimeFrame &&
					adi->frame == NULL && adi->alpha > 0)
				 {
					//простые float массивы
					if(adi->typeres == 0)
					{
					       float zn1 = adi->res[i];
					       float zn2 = adi->res[i + 1];
					       if(zn1 != 0 && zn2 != 0)
					       {
						      // Проверим, чтобы индиакторы не выходили за рамки окна графика
						      float y1 = (sred - (zn1 - avg)*ky);
						      float y2 = (sred - (zn2 - avg)*ky);
						      // Не рисуем, если оба находятся выше или ниже окна графика
						      if((y1 > frame.y + hthead + 30 || y2 > frame.y + hthead + 30) &&
							 (y1 < frame.y + frame.height - 20 || y2 < frame.y + frame.height - 20))
						      {
							      if(y1 < frame.y + hthead + 30)
								     y1 = frame.y + hthead + 30;
							      if(y2 < frame.y + hthead + 30)
								     y2 = frame.y + hthead + 30;
							      if(y1 > frame.y + frame.height - 20)
								     y1 = frame.y + frame.height - 20;
							      if(y2 > frame.y + frame.height - 20)
								     y2 = frame.y + frame.height - 20;
							      Gdiplus::Pen pen(Gdiplus::Color(255, GetRValue(adi->col), GetGValue(adi->col), GetBValue(adi->col)), 2);
							      graphics->DrawLine(&pen, float (t1), y1, float (t1 + wcndl), y2);
						      }
					       }
					}
					//структура float_f3 с 3-мя массивами float
					if(adi->typeres == FLOAT_3)
					{
					       struct float_f3* f3 = (struct float_f3*)adi->res;
					       float zn1 = f3->a[i];
					       float zn2 = f3->a[i + 1];
					       if(zn1 != 0 && zn2 != 0)
					       {
						      Gdiplus::Pen pen(Gdiplus::Color(255, 255, 0, 0), 2);
						      graphics->DrawLine(&pen, float (t1), float (sred - (zn1 - avg)*ky),
									       float (t1 + wcndl), float (sred - (zn2 - avg)*ky) );
					       }
					       zn1 = f3->b[i];
					       zn2 = f3->b[i + 1];
					       if(zn1 != 0 && zn2 != 0)
					       {
						      Gdiplus::Pen pen(Gdiplus::Color(255, 0, 255, 0), 2);
						      graphics->DrawLine(&pen, float (t1), float (sred - (zn1 - avg)*ky),
									       float (t1 + wcndl), float (sred - (zn2 - avg)*ky) );
					       }
					       zn1 = f3->c[i];
					       zn2 = f3->c[i + 1];
					       if(zn1 != 0 && zn2 != 0)
					       {
						      Gdiplus::Pen pen(Gdiplus::Color(255, 0, 0, 255), 2);
						      graphics->DrawLine(&pen, float (t1), float (sred - (zn1 - avg)*ky),
									       float (t1 + wcndl), float (sred - (zn2 - avg)*ky) );
					       }
					}
					//точки
					if(adi->typeres == PRICE_POINTS)
					{
					       float zn1 = adi->res[i];
					       if(zn1 != 0)
					       {
						      bit->Canvas->Brush->Color = adi->col;
						      bit->Canvas->Ellipse(t1 - 3, sred - (zn1 - avg)*ky - 3,
										   t1 + 3, sred - (zn1 - avg)*ky + 3);
					       }
					}
					//точки экстремумов mmlast
					if(adi->typeres == EXTREMUMS_PRICE_POINTS)
					{
					       bit->Canvas->Pen->Color = clWhite;
					       struct indexes_extremums* ie = (struct indexes_extremums*)adi->res;
					       if(ie->indexes[i] > 0)
					       {
						      struct extremum extr = ie->extremums[ie->indexes[i]];

						      adi->col;
						      if(!adi->col)
						      {
							      if(extr.min) bit->Canvas->Brush->Color = RGB(148, 148, 0); //clBlue;
							      else bit->Canvas->Brush->Color = RGB(0, 148, 148); //clGreen;
						      }
						      else
						      {
							      int r = GetRValue(adi->col);
							      int g = GetGValue(adi->col);
							      int b = GetBValue(adi->col);

							      if(extr.min) bit->Canvas->Brush->Color = RGB(Max(r/2, 0), Max(g/2, 0), Max(b/2, 0)); //clBlue;
							      else bit->Canvas->Brush->Color = RGB(Min(r*2, 255), Min(g*2, 255), Min(b*2, 255)); //clGreen;
						      }

						      float x1 = realx[extr.index - realistart];
						      float y1 = RoundTo(sred - (extr.val - avg)*ky, 0);

						      int szextr = 3;
						      if(ww >= 10)
							     szextr = 7;
						      bit->Canvas->Pen->Width = 1;
						      bit->Canvas->Ellipse(Rect(x1 - szextr, y1 - szextr, x1 + szextr, y1 + szextr));

						      // Нарисуем линии выше и ниже экстремумов, чтобы точно знать какие есть в этой точке
						      bit->Canvas->Pen->Color = bit->Canvas->Brush->Color;
						      bit->Canvas->Pen->Width = 3;
						      int psz = StrToInt(adi->mass_params[2]);
						      bit->Canvas->Font->Size = 7;
						      bit->Canvas->Font->Color = bit->Canvas->Brush->Color;
						      SetBkMode(bit->Canvas->Handle, TRANSPARENT);
						      if(extr.min)
						      {
							      bit->Canvas->MoveTo(x1 - szextr, y1 + psz/5);
							      bit->Canvas->LineTo(x1 + szextr, y1 + psz/5);
							      bit->Canvas->TextOut(x1 - szextr, y1 + psz/5 - 10, adi->mass_params[2]);
						      }
						      else
						      {
							      bit->Canvas->MoveTo(x1 - szextr, y1 - psz/5);
							      bit->Canvas->LineTo(x1 + szextr, y1 - psz/5);
							      bit->Canvas->TextOut(x1 - szextr, y1 - psz/5 - 10, adi->mass_params[2]);
						      }
					       }
					}
				 }
				 adi = adi->next;
			  }

			  sttmin = GetTickCount() - sttmin;
			  sttmin5 += sttmin;
			  sttmin = GetTickCount();
		   }            

		   sttm = GetTickCount() - sttm;
		   sttm = GetTickCount();

		   //горизонтальные объемы с начала дня до свечки на позиции курсора иначе до конца видимой части
		   //найдем свечку начала дня
		   int istartday = 0;
		   AnsiString dt = GetDate(cndls[istart].timeopen);
		   for(int i = istart; i >= 0; i--)
		   {
			  AnsiString dt = GetDate(cndls[i].timeopen);
			  if(dt != lastdt) break;
			  istartday = i;
			  lastdt = dt;
		   }
		   //посчитаем кластеры, разделим на кластеры побольше HorKlastHt->Position
		   struct klaster kls;
		   ZeroMemory(&kls, sizeof(struct klaster));
		   int posstop = numcandlempos;
		   if(posstop < 0) posstop = kolvo + istart;
		   for(int i = istartday; i <= posstop && i < maxicandles; i++)
		   {
			  struct klasterdata* kl = klast[i].klasters;
			  if(kl)
			  while(1)
			  {
				 float pr = kl->price/HorKlastHt->Position;
				 int pr2 = pr;
				 pr = pr2*HorKlastHt->Position;
				 kls.AddOrEditKlaster2(pr, kl->volume_bue, kl->volume_sell);

				 if(kl->next) kl = kl->next;
				 else break;
			  }
		   }
		   //ширина кластера в пикс.
		   int wdklast = ky*HorKlastHt->Position;
		   if(wdklast < 1)  wdklast = 1;
		   if(wdklast >= 5) wdklast++;

		   //рисуем гор. объемы
		   struct klasterdata* kl = kls.klasters;
		   if(kl)
		   while(1)
		   {
			  int y1 = RoundTo(sred - (kl->price + 0 - avg)*ky, 0) - wdklast;
			  int y2 = RoundTo(sred - (kl->price - 0 - avg)*ky, 0);

			  if(y1 > frame.y + 30 && y2 < frame.y + frame.height - 30)
			  {
				 bit->Canvas->Brush->Color = RGB(100, 100, 250);
				 bit->Canvas->FillRect(Rect(frame.x + 5, y1,
						  frame.x + 5 + (kl->volume_bue + kl->volume_sell)/10/wdklast/kftmfrm, y2));
				 if(kl->volume_bue >= kl->volume_sell)
				 {
					bit->Canvas->Brush->Color = RGB(100, 200, 100);
					bit->Canvas->FillRect(Rect(frame.x + 5, y1, frame.x + 5 + (kl->volume_bue)/10/wdklast/kftmfrm, y2));
					bit->Canvas->Brush->Color = RGB(200, 100, 100);
					bit->Canvas->FillRect(Rect(frame.x + 5, y1, frame.x + 5 + (kl->volume_sell)/10/wdklast/kftmfrm, y2));
				 }
				 else
				 {
					bit->Canvas->Brush->Color = RGB(200, 100, 100);
					bit->Canvas->FillRect(Rect(frame.x + 5, y1, frame.x + 5 + (kl->volume_sell)/10/wdklast/kftmfrm, y2));
					bit->Canvas->Brush->Color = RGB(100, 200, 100);
					bit->Canvas->FillRect(Rect(frame.x + 5, y1, frame.x + 5 + (kl->volume_bue)/10/wdklast/kftmfrm, y2));
				 }
			  }

			  if(kl->next) kl = kl->next;
			  else break;
		   }

		   sttm = GetTickCount() - sttm;
		   sttm = GetTickCount();

		   //если режим симуляции и свечка мышью не выбрана, то показываем кластеры трех последних свечек в симуляции
		   if((numcandlempos < 0 || curX <= 0 || curY <= 0) && Simulator->Checked)
			  numcandlempos = maxicandles - 1;

		   //свечки с кластерами, покажем count_klast_candles свечки, учитывая текущую, по позиции мыши
		   
		   // удален код

		   sttm = GetTickCount() - sttm;
		   sttm = GetTickCount();

		   //если ставим уровень, вычислим цену
		   if(curSelPrice &&
			  curX > 0 && curX > frame.x + widthklasters &&
			  curY > 0 && ky)
		   {
			  float pricempos = RoundTo(avg + (sred - curY)/ky, 0);
			  if(count_urovn < 10)
			  {
				  urovni[count_urovn] = pricempos;
				  count_urovn++;
			  }
		   }

		   //рисуем уровни
		   if(count_urovn > 0)
		   {
			  bit->Canvas->Font->Color = clBlack;
			  bit->Canvas->Pen->Width = 2;
			  bit->Canvas->Pen->Color = RGB(80, 200, 100);
			  for(int i = 0; i < count_urovn; i++)
			  {
				 //pricempos = avg + (sred - curY)/ky
				 float cy = sredkl - (urovni[i] - avg)*ky;
				 if(cy > frame.y + 20 && cy < frame.y + frame.height - 20)
				 {
					bit->Canvas->MoveTo(frame.x + widthklasters, cy);
					bit->Canvas->LineTo(frame.x + frame.width, cy);

					bit->Canvas->Brush->Color = RGB(250, 250, 250);
					bit->Canvas->TextOutW(frame.x + widthklasters + 10, cy - 10, FloatToStr(RoundTo(urovni[i], 0)));
				 }
			  }
		   }

		   sttm = GetTickCount() - sttm;
		   sttm = GetTickCount();


		   //инфа по координатам мыши
		   //и линия цены на кластерах
		   
		   //удален код

		   sttm = GetTickCount() - sttm;
		   sttm = GetTickCount();

		   //рисуем экстремумы
		   if(count_extremums > 0)
		   {
			  bit->Canvas->Pen->Color = clWhite;
			   for(int i = 0; i < count_extremums; i++)
			   {
				  struct extremum extr = extremums[i];
				  if(extr.index < realistart)
					 continue;
				  if(extr.index > realiend || extr.index > maxicandles - 2)
					 break;

				  if(extr.min) bit->Canvas->Brush->Color = clBlue;
				  else bit->Canvas->Brush->Color = clGreen;

				  float x1 = realx[extr.index - realistart];
				  float y1 = RoundTo(sred - (extr.val - avg)*ky, 0);

				  bit->Canvas->Ellipse(Rect(x1 - 3, y1 - 3, x1 + 3, y1 + 3));
			   }
		   }

		   sttm = GetTickCount() - sttm;
		   sttm = GetTickCount();

		   //рисуем импульсы и боковики
		   if(count_impulses > 0)
		   {
			   bit->Canvas->Pen->Color = clWhite;
			   bit->Canvas->Pen->Width = 1;
			   for(int i = 0; i < count_impulses && i < maxicandles; i++)
			   {
				  bit->Canvas->Brush->Color = clBlack;
				  struct impulse imp = impulses[i];
				  struct candlesdata cndl1 = candles[CurTimeFrame][imp.ind_start];
				  struct candlesdata cndl2 = candles[CurTimeFrame][imp.ind_end];
				  struct candlesdata cndl3 = candles[CurTimeFrame][imp.ind_enter];
				  struct candlesdata cndl4 = candles[CurTimeFrame][imp.ind_zamedl];
				  struct candlesdata cndl5 = candles[CurTimeFrame][imp.ind_closeimp];

				  if(imp.ind_end < istart /*|| imp.ind_start > istart + kolvo*/)
					 continue;

				  if(imp.ind_start > istart + kolvo || imp.ind_end > maxicandles)
					 break;

				  float x1 = frame.x + xsmesh + widthklasters + imp.ind_start*wcndl - xsm;
				  float x2 = frame.x + xsmesh + widthklasters + imp.ind_end*wcndl - xsm;
				  float y1 = RoundTo(sred - (cndl1.priceopen - avg)*ky, 0);
				  float y2 = RoundTo(sred - (cndl2.priceclose - avg)*ky, 0);
				  //возможная точка входа
				  float x3 = frame.x + xsmesh + widthklasters + imp.ind_enter*wcndl - xsm;
				  float y3 = RoundTo(sred - (cndl3.priceclose - avg)*ky, 0);
				  //начало замедления
				  float x4 = frame.x + xsmesh + widthklasters + imp.ind_zamedl*wcndl - xsm;
				  float y4 = RoundTo(sred - (cndl4.priceclose - avg)*ky, 0);
				  //конец импульса по объемам
				  float x5 = frame.x + xsmesh + widthklasters + imp.ind_closeimp*wcndl - xsm;
				  float y5 = RoundTo(sred - (cndl5.priceclose - avg)*ky, 0);

				  bit->Canvas->Ellipse(Rect(x1 - 3, y1 - 3, x1 + 3, y1 + 3));
				  bit->Canvas->Ellipse(Rect(x2 - 3, y2 - 3, x2 + 3, y2 + 3));
				  bit->Canvas->Ellipse(Rect(x3 - 3, y3 - 3, x3 + 3, y3 + 3));

				  if(imp.up) bit->Canvas->Brush->Color = clGreen;
				  else       bit->Canvas->Brush->Color = clBlue;

				  if(y1 <= y2) bit->Canvas->FrameRect(Rect(x1, y1, x2, y2));
				  else         bit->Canvas->FrameRect(Rect(x1, y2, x2, y1));
				  if(imp.ind_zamedl > 0)
				  {
					 bit->Canvas->Brush->Color = clRed;
					 bit->Canvas->Ellipse(Rect(x4 - 3, y4 - 3, x4 + 3, y4 + 3));
				  }
				  if(imp.ind_closeimp > 0)
				  {
					 bit->Canvas->Brush->Color = clBlue;
					 bit->Canvas->Ellipse(Rect(x5 - 3, y5 - 3, x5 + 3, y5 + 3));
				  }

				  //значения суммы
				  bit->Canvas->Brush->Color = clWhite;
				  bit->Canvas->Font->Color  = clBlack;

				  float vl1 = cndls[imp.ind_closeimp].priceclose - cndls[imp.ind_enter].priceclose;
				  float vl2 = cndls[imp.ind_end].priceclose - cndls[imp.ind_enter].priceclose;
				  if(!imp.up)
				  {
					 vl1 = cndls[imp.ind_enter].priceclose - cndls[imp.ind_closeimp].priceclose;
					 vl2 = cndls[imp.ind_enter].priceclose - cndls[imp.ind_end].priceclose;
				  }

				  TSize sz1 = bit->Canvas->TextExtent(FloatToStr(vl1) + "; " + FloatToStr(vl2));
				  bit->Canvas->TextOutW((x1 + x2)/2 - sz1.cx/2, y1 - 20,
										FloatToStr(vl1) + "; " + FloatToStr(vl2));
			   }

		   }

		   sttm = GetTickCount() - sttm;
		   sttm = GetTickCount();

		   // Сделки по анализу
		   if(SmartTest && SmartTest->count_orders > 0)
		   {
			   bit->Canvas->Pen->Color = clWhite;
			   for(int i = 0; i < SmartTest->count_orders; i++)
			   {
				  bit->Canvas->Brush->Color = clBlack;
				  struct iorders ord = SmartTest->orders[i];

				  //находим индекс, соответствующий текущему
				  int ind1 = 1 + candles[SmartTest->taimframe_orders][ord.id_candle1].indexs[CurTimeFrame];
				  int ind2 = 1 + candles[SmartTest->taimframe_orders][ord.id_candle2].indexs[CurTimeFrame];

				  if(ind1 < 0 || ind2 < 0 || ind1 >= count_candles[SmartTest->taimframe_orders] || ind2 >= count_candles[SmartTest->taimframe_orders] )
					  continue;

				  if( (ind1 < realistart || ind1 > realiend) &&
					  (ind2 < realistart || ind2 > realiend) && !(ind1 < istart && ind2 > realiend))
					  continue;

				  float x1 = realx[ind1 - realistart];
				  float x2 = realx[ind2 - realistart];
				  float y1 = RoundTo(sred - (ord.price1 - avg)*ky, 0);
				  float y2 = RoundTo(sred - (ord.price2 - avg)*ky, 0);
				  if(x2 == 0)
					 x2 = frame.x + frame.width;

				  bit->Canvas->Ellipse(Rect(x1 - 3, y1 - 3, x1 + 3, y1 + 3));
				  bit->Canvas->Ellipse(Rect(x2 - 3, y2 - 3, x2 + 3, y2 + 3));

				  // Цвет рамки
				  bit->Canvas->Brush->Color = ord.col;
				  if(ord.col == clGreen)
					 int aa = 11;

				  // Выделенную сделку сделаем жирнее
				  if(ord.id_candle1 == CurSelIndexCandles + 0 )
				  {       
					  bit->Canvas->Brush->Color = clFuchsia;
				  }
				  else bit->Canvas->Pen->Width = 1;

				  if(y1 <= y2) bit->Canvas->FrameRect(Rect(x1 + 1, y1, x2 + 1, y2));
				  else         bit->Canvas->FrameRect(Rect(x1 + 1, y2, x2 + 1, y1));

				  if(y1 <= y2) bit->Canvas->FrameRect(Rect(x1 + 2, y1 + 1, x2 + 2, y2));
				  else         bit->Canvas->FrameRect(Rect(x1 + 2, y2 - 1, x2 + 2, y1));

				  if(ord.id_candle1 == CurSelIndexCandles + 0)
				  {
					  if(y1 <= y2) bit->Canvas->FrameRect(Rect(x1 + 3, y1 + 2, x2 + 3, y2));
					  else         bit->Canvas->FrameRect(Rect(x1 + 3, y2 - 2, x2 + 3, y1));
				  }

				  double sum = ord.price1 - ord.price2;
				  if(ord.up) sum = ord.price2 - ord.price1;
				  if(fabs(sum) < 1) sum = RoundTo(sum, -2);
				  if(fabs(sum) < 10 && fabs(sum) >= 1) sum = RoundTo(sum, -1);
				  else sum = RoundTo(sum, 0);

				  AnsiString textsum = FloatToStr(sum);

				  if(ord.id_candle1 == CurSelIndexCandles + 0)
				  {
					  bit->Canvas->Brush->Color = clPurple;
					  bit->Canvas->Font->Color  = clWhite;
					  bit->Canvas->Pen->Width = 4;
					  textsum = " " + textsum + " ";
				  }
				  else
				  {
					  bit->Canvas->Brush->Color = clWhite;
					  bit->Canvas->Font->Color  = RGB(60, 60, 60);
					  bit->Canvas->Pen->Width = 2;
				  }
				  TSize sz1 = bit->Canvas->TextExtent(textsum);
				  bit->Canvas->TextOutW((x1 + x2)/2 - sz1.cx/2, y1 - 10, textsum);

				  if(ord.up) bit->Canvas->Pen->Color = clGreen;
				  else       bit->Canvas->Pen->Color = clRed;
				  bit->Canvas->MoveTo((x1 + x2)/2 - sz1.cx/2, y1 - 10);
				  bit->Canvas->LineTo((x1 + x2)/2 + sz1.cx/2, y1 - 10);
			   }

			   sttm = GetTickCount() - sttm;
			   sttm = GetTickCount();

			   //экстремумы
			   if(SmartTest->count_pricepoints > 0)
			   {
				   bit->Canvas->Pen->Width = 1;
				   bit->Canvas->Pen->Color   = clWhite;
				   bit->Canvas->Brush->Color = TColor RGB(0, 127, 127);
				   for(int i = 0; i < SmartTest->count_pricepoints; i++)
				   {
					  //находим индекс, соответствующий текущему
					  int ind1 = 1 + candles[SmartTest->taimframe_orders][SmartTest->pricepoints[i].index].indexs[CurTimeFrame];

					  if(ind1 < realistart || ind1 > realiend /*istart + kolvo*/)
						  continue;

					  //float x1 = frame.x + xsmesh + widthklasters + ind1*wcndl - xsm;
					  float x1 = realx[ind1 - realistart];
					  float y1 = RoundTo(sred - (SmartTest->pricepoints[i].price - avg)*ky, 0);

					  bit->Canvas->Ellipse(Rect(x1 - 3, y1 - 3, x1 + 3, y1 + 3));
				   }
			   }
		    }

	       }
        }

        // Дни недели и часы для показа на графике
        AnsiString daysofweek[7] = {"пн", "вт", "ср", "чт", "пт", "сб", "вс"};
	AnsiString hoursofday[14] = {"10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23"};

	int wdd = 20;
	int wdh = 18;
	int szbx = 15;
	int xd = frame.x + frame.width - 30 - wdd*7 - wdh*14;
	int yd = 3; 
	bit->Canvas->Font->Size = 10;
	bit->Canvas->Font->Color = RGB(100, 120, 170);
	bit->Canvas->Pen->Color = RGB(120, 170, 230);
        bit->Canvas->Pen->Width = 1;
	bit->Canvas->Brush->Color = clWhite;
	SetBkMode(bit->Canvas->Handle, TRANSPARENT);
	for(int i = 0; i < 7; i++)
	{
		int xx  = xd + i*wdd;
		int wdx = wdd - 2;
		if(frame.show_daysofweek[i].show)
		{
			bit->Canvas->Brush->Color = RGB(0, 170, 120);
			bit->Canvas->Font->Color = clWhite;
			bit->Canvas->Pen->Color = RGB(80, 160, 110);
		}
		else
		{
			bit->Canvas->Brush->Color = clWhite;
			bit->Canvas->Font->Color = RGB(100, 120, 170);
			bit->Canvas->Pen->Color = RGB(120, 170, 230);
		}

		SetBkMode(bit->Canvas->Handle, TRANSPARENT);
		bit->Canvas->Rectangle(Rect(xx, yd, xx + wdx, yd + szbx));
		bit->Canvas->TextOut(xx + 2, yd - 2, daysofweek[i]);
		frame_->show_daysofweek[i].x  = xx;
		frame_->show_daysofweek[i].wd = wdx;
		frame_->show_daysofweek[i].y  = yd;
		frame_->show_daysofweek[i].ht = szbx;
	}
	bit->Canvas->Font->Size = 8;
	xd += 7*wdd + 10;
	for(int i = 0; i < 14; i++)
	{
		int xx  = xd + i*wdh;
		int wdx = wdh - 2;
		if(frame.show_hoursofday[i].show)
		{
			bit->Canvas->Brush->Color = RGB(0, 150, 200);
			bit->Canvas->Font->Color = clWhite;
			bit->Canvas->Pen->Color = RGB(80, 140, 190);
		}
		else
		{
			bit->Canvas->Brush->Color = clWhite;
			bit->Canvas->Font->Color = RGB(100, 120, 170);
			bit->Canvas->Pen->Color = RGB(120, 170, 230);
		}

		SetBkMode(bit->Canvas->Handle, TRANSPARENT);
		bit->Canvas->Rectangle(Rect(xx, yd, xx + wdh - 2, yd + szbx));
		bit->Canvas->TextOut(xd + i*wdh + 2, yd + 1, hoursofday[i]);
		frame_->show_hoursofday[i].x  = xx;
		frame_->show_hoursofday[i].wd = wdx;
		frame_->show_hoursofday[i].y  = yd;
		frame_->show_hoursofday[i].ht = szbx;
	}

	CurSearchIndexCandles = 0;
}
//---------------------------------------------------------------------------

//удален код

//---------------------------------------------------------------------------
void __fastcall TFMain::DrawMultiHistogram(struct frames frame)
{
       if(!frame.visible || !frame.multidata || frame.count_data <= 0 ||
               frame.count_data2 <= 0 || !frame.multidata[0])
	              return;

       unsi wcndl = 40;
       if(frame.weight > 0)
	      wcndl = frame.weight;

       int count_data = frame.count_data;
       int count_data2 = frame.count_data2;
       float** data = (float**)frame.multidata;

       int xsmesh = 10 + wcndl/2;

       if(frame.AutoWidth)
       {
	       frame.width = count_data*wcndl + 10 + xsmesh*2;
	       if(frame.width < 200)
               frame.width = 200;
	       frame.needwidth = frame.width;

	       if(frame.x + frame.width > ClientWidth)
		      frame.width = ClientWidth - frame.x;

       }

       int kf = ZoomCandles->Position; 
       if((frame.width - 100) != 0 && count_data*wcndl / (frame.width - 100) > kf)
	      kf = count_data*wcndl / (frame.width - 100);
       int istart = frame.xscrollpos*kf/wcndl + frame.xsmallscrollpos;
       int xsm = istart*wcndl;

       float sred = frame.y + 20 + (frame.height - 45)/2;
       //кол-во свечек на видимой части графика
       int kolvo = (frame.width - xsmesh*2)/wcndl;
       float min = data[frame.smesh_1][frame.smesh_2], max = data[frame.smesh_1][frame.smesh_2];
       for(int i = 0; i < count_data; i++)
       {
	       int ii = i + frame.smesh_1;
	       for(int j = 0; j < count_data2; j++)
	       {
		       int jj = j + frame.smesh_2;
		       if(data[ii][jj] < min) min = data[ii][jj];
		       if(data[ii][jj] > max) max = data[ii][jj];
	       }
       }
       float ky  = 0;
       if(max - Min((float)min, (float)0.0) != 0)             //высота заголовка, высота скролла
	      ky = ((float)frame.height - 20.0 - 20.0 - 45.0)/(max - Min((float)min, (float)0.0));

       float avg = (Min((float)min, (float)0.0) + max)/2.0;

       int nulliney = RoundTo(sred - (0 - avg)*ky, 0);

       DrawWind(frame);

       int wg = wcndl*0.9;
       float wdh = (float)wg/(float)count_data2;

       bit->Canvas->Pen->Color = clWhite;
       bit->Canvas->Pen->Width = 1;
       //нулевая линия
       bit->Canvas->MoveTo(frame.x + 2, nulliney);
       bit->Canvas->LineTo(frame.x + frame.width - 2, nulliney);

       bit->Canvas->Font->Name = "Tahoma";
       bit->Canvas->Font->Size = 7;
       bit->Canvas->Font->Style = TFontStyles();
       bit->Canvas->Font->Color = RGB(0, 100, 200);

       bit->Canvas->Pen->Color = RGB(100, 100, 100);
       SetBkMode(bit->Canvas->Handle, TRANSPARENT);

       // Список цветов
       TColor colshistogr[40];
       colshistogr[0] = clGreen;
       colshistogr[1] = RGB(0, 210, 50);
       colshistogr[2] = RGB(0, 230, 230);
       colshistogr[3] = RGB(0, 150, 250);
       colshistogr[4] = RGB(0, 100, 250);
       colshistogr[5] = RGB(255, 200, 0);
       colshistogr[6] = RGB(255, 100, 50);
       colshistogr[7] = RGB(200, 80, 255);
       colshistogr[8] = RGB(150, 150, 150);
       colshistogr[9] = RGB(255, 255, 50);
       colshistogr[10] = clTeal;
       colshistogr[11] = clAqua;

       int endhistogr = 0;
       for(int i = istart; i <= istart + kolvo && i < count_data; i++)
       {
	       for(int j = 0; j < count_data2; j++)
	       {
		       float zndt = data[i + frame.smesh_1][j + frame.smesh_2];

		       if(zndt > 0)
			       bit->Canvas->Pen->Color = clGreen;
		       else
		       {
			       bit->Canvas->Pen->Color = clRed;
		       }

		       int xst = frame.x + xsmesh + i*wcndl - wg/2 - xsm + wdh*j;
			 
		       if(data[i] > 0)
			      bit->Canvas->Brush->Color = colshistogr[j];
		       else
			      bit->Canvas->Brush->Color = clRed;

		       bit->Canvas->FillRect(Rect(xst, RoundTo(sred - (0 - avg)*ky, 0),
										   xst + wdh, RoundTo(sred - (zndt - avg)*ky, 0)));
	        }

                if(wdh >= 9)
	        {
		        SetBkMode(bit->Canvas->Handle, TRANSPARENT);
			//округляем значение, исходя из его величины
			double zn = RoundSmart(zndt);
			// Форматируем число, если оно больше 999, чтобы оно было читаемым
			AnsiString txt = FormatDigit(zn);

			bit->Canvas->Font->Color = RGB(0, 150, 150);
			TSize sz1 = bit->Canvas->TextExtent(txt);
			int y = RoundTo(sred - (zndt - avg)*ky, 0) - 15;
			if(zndt <= 0) y = RoundTo(sred - (0 - avg)*ky, 0) - 15;
			bit->Canvas->TextOutW(xst - sz1.cx/2, y, txt);
	        }
       }
   
       AnsiString podp = "";
       if(frame.podpisi && frame.podpisi[i])
	      podp = (AnsiString)frame.podpisi[i];

       SetBkMode(bit->Canvas->Handle, TRANSPARENT);

       bit->Canvas->Font->Color = RGB(0, 100, 150);
       TSize sz1 = bit->Canvas->TextExtent(podp);
       bit->Canvas->TextOutW(frame.x + xsmesh + i*wcndl - sz1.cx/2 - xsm, frame.y + frame.height - 35, podp);
       endhistogr = frame.x + xsmesh + i*wcndl - sz1.cx/2 - xsm + sz1.cx;


       int ym = 15;
       if(count_data2 > 7)
	      ym = 10;
       if(count_data2 > 11)
	      ym = 9;
	  
       for(int j = 0; j < count_data2; j++)
       {
	       int y = frame.y + 40 + j*ym;
	       int x1 = endhistogr + 20;
	       int x2 = endhistogr + 40;
	       int x3 = endhistogr + 50;
	       if(x1 > frame.x + frame.width)
		      x1 = frame.x + frame.width;
	       if(x2 > frame.x + frame.width)
		      x2 = frame.x + frame.width;
	       if(x1 > frame.x)
	       {
		       bit->Canvas->Brush->Color = colshistogr[j];
		       bit->Canvas->Font->Color  = colshistogr[j];
		       bit->Canvas->FillRect(Rect(x1, y, x2, y + 5));
	       }

	       if(x3 < frame.x + frame.width - 10 && x3 > frame.x)
	       {
		       SetBkMode(bit->Canvas->Handle, TRANSPARENT);
		       bit->Canvas->TextOutW(endhistogr + 50, y - 4, (AnsiString)frame.podpisi2[j]); //frame.podpisi2[j];
	       }
       }
}
//---------------------------------------------------------------------------

//удален код

//---------------------------------------------------------------------------
// Вычислим минуты часы дни месяцы годы для оптимизации скорости анализа
void __fastcall TFMain::CalcMinHourDayMonthYear()
{
	__int64 starttime = GetTickCount();

	double secofday = (double)(24*3600);
        int ch = 0, ch2 = 0;

	TDateTime* dt = new TDateTime(1970,1,1,0,0,0,0);    //+3 часа - московское время
        double dtval = dt->Val;

	for(int i = 0; i < 10; i++)
	{
		if(!candles[i])
		   continue;

		int ct_candles = count_candles[i];

		__int64 curtimeopen = 0;
		unsi curnummin = 0;
		unsi curnumhour = 0;
		unsi curnumdayofweek = 0;
		unsi curnumdaym = 0;
		unsi curnummonth = 0;
		short int curnumyear = 0;
		for(int c = 0; c < ct_candles; c++)
		{
			__int64 tmopen = candles[i][c].timeopen;

			struct timedaymonthyear_candles* tmdmy = &tmdmy_candles[i][c];

			// Оптимизируем для 1,5,10,15,30 минутных свечек, их больше всего. Не обязательно всё считать каждую минуту
			if((i == 0 || i == 1 || i == 2 || i == 3 || i == 4 || i == 5 /*|| i == 6*/) && c > 0 && curtimeopen > 0)
			{
				__int64 difftm = tmopen - curtimeopen;
				if(difftm < 3600)
				{
					tmdmy->minute     = curnummin + difftm/60;
					tmdmy->hour       = curnumhour;
					tmdmy->dayofweek  = curnumdayofweek;
					tmdmy->dayofmonth = curnumdaym;
					tmdmy->month      = curnummonth;
					tmdmy->year       = curnumyear;

                                        continue;
                                }
				if(difftm < 7200 && curnumhour < 24)
				{
					difftm -= 3600;
					tmdmy->minute     = curnummin + difftm/60;
					tmdmy->hour       = curnumhour + 1;
					tmdmy->dayofweek  = curnumdayofweek;
					tmdmy->dayofmonth = curnumdaym;
					tmdmy->month      = curnummonth;
					tmdmy->year       = curnumyear;

                                        continue;
				}
				if(difftm < 10800 && curnumhour < 23)
				{
					difftm -= 7200;
					tmdmy->minute     = curnummin + difftm/60;
					tmdmy->hour       = curnumhour + 2;
					tmdmy->dayofweek  = curnumdayofweek;
					tmdmy->dayofmonth = curnumdaym;
					tmdmy->month      = curnummonth;
					tmdmy->year       = curnumyear;

                                        continue;
				}
			}
			ch++;

			dt->Val = dtval + (double)(tmopen) / secofday;

			AnsiString res = dt->FormatString("nn");
			unsi nummin = StrToInt(res) - 0;

			res = dt->FormatString("hh");
			unsi numhour = StrToInt(res) - 0;

			unsi numdayofweek, numdaym, nummonth;
			short int numyear;
			if(curtimeopen > 0 && tmopen - curtimeopen <= 3600*(numhour - curnumhour) && numhour > curnumhour)
			{
				tmdmy->minute     = nummin;
				tmdmy->hour       = numhour;
				tmdmy->dayofweek  = curnumdayofweek;
				tmdmy->dayofmonth = curnumdaym;
				tmdmy->month      = curnummonth;
				tmdmy->year       = curnumyear;

				if(nummin == 0)
				{
					curtimeopen = tmopen;
					curnummin       = nummin;
					curnumhour      = numhour;
				}
                                ch2++;
			}
			else
			{
				numdayofweek = dt->DayOfWeek();
				if(numdayofweek == 1)           //воскресенье
				   numdayofweek = 8;
				numdayofweek -= 1;

				res = dt->FormatString("dd");
				 numdaym = StrToInt(res) - 0;

				res = dt->FormatString("mm");
				 nummonth = StrToInt(res) - 0;

				res = dt->FormatString("yyyy");
				 numyear = StrToInt(res) - 0;

				tmdmy->minute     = nummin;
				tmdmy->hour       = numhour;
				tmdmy->dayofweek  = numdayofweek;
				tmdmy->dayofmonth = numdaym;
				tmdmy->month      = nummonth;
				tmdmy->year       = numyear;


				if(nummin == 0)
				{
					curtimeopen = tmopen;
					curnummin       = nummin;
					curnumhour      = numhour;
					curnumdayofweek = numdayofweek;
					curnumdaym      = numdaym;
					curnummonth     = nummonth;
					curnumyear      = numyear;
				}
			}
               }
	}
       delete dt;    ch;  ch2;
       starttime = GetTickCount() - starttime;
}
//---------------------------------------------------------------------------
void __fastcall TFMain::FormMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y)
{
       if(Button == mbRight) 
          pushright = true;

       unsi wdvertscroll = 20;

       //поправка по скроллу
       int pp = ScrollBar1->Position;  Y -= 60;

       //найдем фрейм
       for(int i = 0; i < count_frames; i++)
       {
	      if(iframes[i].visible)
	      if(X >= iframes[i].x && X <= iframes[i].x + iframes[i].width &&
		 Y + pp >= iframes[i].y && Y + pp <= iframes[i].y + iframes[i].height)
	      {
		   // Разрешенные дни недели и часы для TYPE_PRICE_CANDLES
		   if(iframes[i].typedata == TYPE_PRICE_CANDLES)
		   {
			   if(iframes[i].ClickShow(X, Y))
			   {
				   DrawAll();
				   return;
			   }
                   }
		   // Разрешенные годы для TYPE_PRICE_CANDLES
		   if(iframes[i].typedata == TYPE_ORDERS_1)
		   {
			   if(iframes[i].ClickYearShow(X, Y))
			   {
				   DrawAll();
				   return;
			   }
		   }

		   //двигаем гор.скролл
		   if(!pushright && X >= iframes[i].x && X <= iframes[i].x + iframes[i].width - 20 - wdvertscroll &&
			  Y + pp >= iframes[i].y + iframes[i].height - 15 && Y + pp <= iframes[i].y + iframes[i].height)
		   {
			  curframe = &iframes[i];
			  movegorscroll = true;
			  lastX = X;
			  lastY = Y;
			  if(X < curframe->x + curframe->xscrollpos ||
				 X > curframe->x + curframe->xscrollpos + 100)
				 curframe->xscrollpos = (X - curframe->x - 50)*1; //X - curframe->x;
			  if(curframe->xscrollpos < 0) curframe->xscrollpos = 0;
			  DrawAll();
		   }
                   //двигаем малый гор.скролл
		   if(pushright && X >= iframes[i].x && X <= iframes[i].x + iframes[i].width - 20 - wdvertscroll &&
			  Y + pp >= iframes[i].y + iframes[i].height - 15 && Y + pp <= iframes[i].y + iframes[i].height)
		   {
			  curframe = &iframes[i];
			  movegorscroll = true;
			  lastX = X;
			  lastY = Y;
			  if(curframe->xsmallscrollpos < 0) curframe->xsmallscrollpos = 0;
			  DrawAll();
		   }
		   //вправо или влево на пиксель (свечку)
		   if(X >= iframes[i].x + iframes[i].width - 20 - wdvertscroll && X <= iframes[i].x + iframes[i].width &&
			  Y + pp >= iframes[i].y + iframes[i].height - 20 && Y + pp <= iframes[i].y + iframes[i].height)
		   {
			  curframe = &iframes[i];

			  if(curframe->xsmallscrollpos < 0) curframe->xsmallscrollpos = 0;
			  //DrawAll();

			  if(X < iframes[i].x + iframes[i].width - 10 - wdvertscroll)
				 curframe->xscrollpos --;
			  else curframe->xscrollpos ++;

			  //запустим таймер генератор событий нажатия на стрелку, для многократного нажатия при удержании кнопки мыши, как на клавишу
			  if(curframe->xscrollpos < 0)
			  {
				  curframe->xscrollpos = 0;
				  TimerStrelki->Enabled = false;
			  }
			  else
			  {
				  TimerStrelki->Interval = 500;
				  TimerStrelki->Enabled = true;
			  }
			  TSX = X;
			  TSY = Y;

			  if(curframe->xsmallscrollpos < 0)  curframe->xsmallscrollpos = 0;
	                  if(curframe->xsmallscrollpos > 75) curframe->xsmallscrollpos = 75;

			  if(AutoSynhronize->Checked &&
			     curframe->typedata != TYPE_HISTOGRAM_1 &&
			     curframe->typedata != TYPE_HISTOGRAM_2 &&
			     curframe->typedata != TYPE_MULTI_HISTOGRAM_1 &&
			     curframe->typedata != TYPE_ORDERS_1 &&
			     curframe->typedata != TYPE_PRICE_CSIMPLE)
			  {
				 for(int i = 0; i < count_frames; i++)
				 {
					if(iframes[i].typedata != TYPE_HISTOGRAM_1 &&
					   iframes[i].typedata != TYPE_HISTOGRAM_2 &&
					   iframes[i].typedata != TYPE_MULTI_HISTOGRAM_1 &&
					   iframes[i].typedata != TYPE_ORDERS_1 &&
					   iframes[i].typedata != TYPE_PRICE_CSIMPLE)
					{
						iframes[i].xscrollpos      = curframe->xscrollpos;
					        iframes[i].xsmallscrollpos = curframe->xsmallscrollpos;
					}
				 }
			  }

			  DrawAll();
		   }

		   //нажимаем и ставим уровень
		   if(X >= iframes[i].x && X <= iframes[i].x + iframes[i].width &&
		      Y >= iframes[i].y + 20 && Y <= iframes[i].y + iframes[i].height - 20 &&
			  curSetUroven && iframes[i].typedata == TYPE_PRICE_CANDLES)
		   {
			   curframe = &iframes[i];
			   curSelPrice = true;
			   curX = X;
			   curY = Y;
			   DrawAll();
			   curSetUroven = false;
			   curSelPrice  = false;
			   ButLine->Caption = "---";
			   return;
                   }

		   //если нажимаем на элемент внутри фрейма, например на сделку
		   if(X >= iframes[i].x && X <= iframes[i].x + iframes[i].width &&
			  Y >= iframes[i].y + 20 && Y <= iframes[i].y + iframes[i].height - 20)
		   {
			   curframe = &iframes[i];
		   }
		   if(curframe && curframe->typedata == TYPE_ORDERS_1 &&
			  X >= iframes[i].x && X <= iframes[i].x + iframes[i].width &&
			  Y >= iframes[i].y + 20 && Y <= iframes[i].y + iframes[i].height - 20)
		   {
			   unsi ww = ZoomCandles->Position;
                           ww = 30;
			   int num = (X - 20 + curframe->xscrollpos*ww + curframe->xsmallscrollpos - iframes[i].x)/ww;
			   curSelNumOrder = num;

			   int numsi = 0;
			   struct frame_orders* fo = curframe->fo;
			   if(fo)
			   {
				   for(int f = 0; f < 200; f++)
				   {
					   int xx = X - /*20 - */iframes[i].x;
					   if(xx > fo[f].x && xx <= fo[f].x + fo[f].wd)
					   {
						   numsi = fo[f].id_candle;
						   break;
					   }
                                   }
			   }

			   CurSelIndexCandles = numsi;
			   CurSearchIndexCandles = numsi;
			   curframe = &iframes[0];
			   DrawAll();
			   CurSearchIndexCandles = numsi;
                           curframe = &iframes[i];
			   DrawAll();
			   
			   return;
		   }

		   if(AutoSynhronize->Checked && curframe &&
		      curframe->typedata != TYPE_HISTOGRAM_1 &&
		      curframe->typedata != TYPE_HISTOGRAM_2&&
		      curframe->typedata != TYPE_MULTI_HISTOGRAM_1 &&
		      curframe->typedata != TYPE_ORDERS_1 &&
		      curframe->typedata != TYPE_PRICE_CSIMPLE)
		   {
			  for(int i = 0; i < count_frames; i++)
			  {
				 if(iframes[i].typedata != TYPE_HISTOGRAM_1 &&
				    iframes[i].typedata != TYPE_HISTOGRAM_2 &&
				    iframes[i].typedata != TYPE_MULTI_HISTOGRAM_1 &&
				    iframes[i].typedata != TYPE_ORDERS_1 &&
				    iframes[i].typedata != TYPE_PRICE_CSIMPLE)
				 {
					iframes[i].xscrollpos      = curframe->xscrollpos;
					iframes[i].xsmallscrollpos = curframe->xsmallscrollpos;
                                 }
			  }
                   }
            }
      }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::FormMouseMove(TObject *Sender, TShiftState Shift, int X, int Y)
{
       int pp = 0; Y -= 60;

       MoveIntoFramePriceCandles = false;

       if(movegorscroll && curframe)
       {
	      if(!pushright) curframe->xscrollpos      += (X - lastX)*1;
	      if(pushright)  curframe->xsmallscrollpos += (X - lastX)*1;

	      lastX = X; 
	      if(curframe->xscrollpos < 0)      
	             curframe->xscrollpos = 0;
	      if(curframe->xsmallscrollpos < 0) 
	             curframe->xsmallscrollpos = 0;
	      if(curframe->xsmallscrollpos > curframe->width - 70) 
	             curframe->xsmallscrollpos = curframe->width - 70;

	      if(AutoSynhronize->Checked &&
	         curframe->typedata != TYPE_HISTOGRAM_1 &&
	         curframe->typedata != TYPE_HISTOGRAM_2 &&
	         curframe->typedata != TYPE_MULTI_HISTOGRAM_1 &&
	         curframe->typedata != TYPE_ORDERS_1 &&
	         curframe->typedata != TYPE_PRICE_CSIMPLE)
	      {
		     for(int i = 0; i < count_frames; i++)
		     {
			     if(iframes[i].typedata != TYPE_HISTOGRAM_1 &&
			        iframes[i].typedata != TYPE_HISTOGRAM_2 &&
			        iframes[i].typedata != TYPE_MULTI_HISTOGRAM_1 &&
			        iframes[i].typedata != TYPE_ORDERS_1 &&
			        iframes[i].typedata != TYPE_PRICE_CSIMPLE)
			     {
				    iframes[i].xscrollpos      = curframe->xscrollpos;
				    iframes[i].xsmallscrollpos = curframe->xsmallscrollpos;
                             }
		     }
	      }

	      DrawAll();
       }
       else
       {
	   //удален код
       }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::FormMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y)
{
       movegorscroll = false;
       pushright = false;
       curframe  = NULL;
       TimerStrelki->Enabled = false;
}
//---------------------------------------------------------------------------

//удален код

//---------------------------------------------------------------------------
void __fastcall TFMain::StartOrder(int id_candle, float price, bool up)
{
       SmartTest->orders[SmartTest->count_orders].id_candle1 = id_candle;
       SmartTest->orders[SmartTest->count_orders].price1     = price;
       SmartTest->orders[SmartTest->count_orders].up         = up;
       SmartTest->taimframe_orders = CurTimeFrame;       //номер таймфрейма
}
//---------------------------------------------------------------------------
void __fastcall TFMain::EndOrder(int id_candle, float price)
{
       SmartTest->orders[SmartTest->count_orders].id_candle2 = id_candle;
       SmartTest->orders[SmartTest->count_orders].price2     = price;
       SmartTest->count_orders++;
}
//---------------------------------------------------------------------------
void __fastcall TFMain::SimCalcSums()
{
       struct candlesdata cndl = candles[CurTimeFrame][SimulatorPos - 1];
       if(typeOrder == "down")
       {
	      //проверим стоп-лосс сначала
	      if(cndl.high >= priceStopLoss)
	      {
		     totalSum += priceOrder - priceStopLoss;
		     typeOrder     = "";
		     curMarga      = 0;
		     priceOrder    = 0;

		     EndOrder(SimulatorPos - 1, priceStopLoss);
		     priceStopLoss = 0;
	      }

	      //проверим тейк-профит
	      if(cndl.low <= priceTakeProfit)
	      {
		     totalSum += priceOrder - priceTakeProfit;
		     typeOrder     = "";
		     curMarga      = 0;
		     priceOrder    = 0;
		     priceStopLoss = 0;

		     EndOrder(SimulatorPos - 1, priceTakeProfit);
		     priceTakeProfit = 0;
	      }

	      if(typeOrder != "")
		     curMarga = priceOrder - cndl.priceclose;
       }
       if(typeOrder == "up")
       {
	      //проверим стоп-лосс сначала
	      if(cndl.low <= priceStopLoss)
	      {
		     totalSum += priceStopLoss - priceOrder;
		     typeOrder = "";
		     curMarga = 0;
		     priceOrder = 0;

		     EndOrder(SimulatorPos - 1, priceStopLoss);
		     priceStopLoss = 0;
	      }

	      //проверим тейк-профит
	      if(cndl.high >= priceTakeProfit)
	      {
		     totalSum += priceTakeProfit - priceOrder;
		     typeOrder     = "";
		     curMarga      = 0;
		     priceOrder    = 0;
		     priceStopLoss = 0;

		     EndOrder(SimulatorPos - 1, priceTakeProfit);
		     priceTakeProfit = 0;
	      }

	      if(typeOrder != "")
		     curMarga = cndl.priceclose - priceOrder;
       }
       Itogo->Caption = "Итого: " + IntToStr((int)totalSum);
       Marga->Caption = "Маржа: " + IntToStr((int)curMarga);
}
//---------------------------------------------------------------------------
void __fastcall TFMain::ekstrBot1Click(TObject *Sender)
{
       if(!Simulator->Checked) return;
       if(ekstrBot1->Font->Style.Contains(fsBold)) return;
       if(count_idata <= 0)
       {
	      ShowMessage("Нет данных");
	      return;
       }
       ekstrBot1->Font->Style = TFontStyles() << fsBold;
}
//---------------------------------------------------------------------------
void __fastcall TFMain::Button11Click(TObject *Sender)
{
       totalSum = 0;
       curMarga = 0;
       Itogo->Caption = "Итого: " + IntToStr((int)totalSum);
       Marga->Caption = "Маржа: " + IntToStr((int)curMarga);

       //импульс - первая свечка резкое движение, следующая свечка не перекрывет откатом половину первой,
       //следующие свечки не перекрывают откатом половину максимальной прибыли
       //импульс не может быть меньше 300 пунктов
       count_impulses = 0;
       ZeroMemory(impulses, sizeof(struct impulse)*10000);

       int ind_impuls_st = -1;
       struct candlesdata* cndl2 = NULL;
       float d = 0;
       bool up;
       float minpriceclose = 0;
       float maxpriceclose = 0;
       int ind_enter = 0;
       float totalsum2 = 0;

       for(int i = 0; i < count_candles[CurTimeFrame]; i++)
       {
	      struct candlesdata cndl  = candles[CurTimeFrame][i];

	      if(Not10hour->Checked && GetTime(cndl.timeopen) == "10:00:00")
	             continue;

	      struct candlesdata cndl1;
	      if(i > 0) cndl1 = candles[CurTimeFrame][i - 1];
	      if(ind_impuls_st < 0)
	      {
		     if(cndl.priceclose - cndl.priceopen >= 60)
		     {
			    ind_impuls_st = i;
			    cndl2 = &(candles[CurTimeFrame][ind_impuls_st]);
			    d = cndl.priceclose - cndl.priceopen;
			    up = true;
			    maxpriceclose = 0;
			    ind_enter = 0;
		     }
		     if(cndl.priceopen - cndl.priceclose >= 60)
		     {
			    ind_impuls_st = i;
			    cndl2 = &(candles[CurTimeFrame][ind_impuls_st]);
			    d = cndl.priceopen - cndl.priceclose;
			    up = false;
			    minpriceclose = 0;
			    ind_enter = 0;
		     }
	      }
	      else
	      {
		     //возможная точка входа в сделку
		     if(up && cndl.priceclose - cndl2->priceopen >= 300 && !ind_enter)
			    ind_enter = i;

		     //возможная точка входа в сделку
		     if(!up && cndl2->priceopen - cndl.priceclose >= 300 && !ind_enter)
			    ind_enter = i;

		     //ищем замедления - начало отскока
		     //маленькая свечка или закрытие в другую сторону от открытия и направление импульса
                     //можно с объемом попробовать, маленький объем
		     if(up && !impulses[count_impulses].ind_zamedl &&
			(cndl.priceclose < cndl.priceopen + 30 ))
		           impulses[count_impulses].ind_zamedl = i;
		  
		     if(!up && !impulses[count_impulses].ind_zamedl &&
			(cndl.priceopen < cndl.priceclose + 30 ))
		           impulses[count_impulses].ind_zamedl = i;

		     //закроем сделку, если нужно после 23:30
		     if(CloseOrder23_30->Checked && !impulses[count_impulses].ind_closeimp && ind_enter > 0 &&
			GetTime(cndl.timeopen) == "23:30:00")
		     {
                            impulses[count_impulses].ind_closeimp = i;
			    if(up) totalsum2 += cndl.priceclose - candles[CurTimeFrame][ind_enter].priceclose;
			    else   totalsum2 += candles[CurTimeFrame][ind_enter].priceclose - cndl.priceclose;
                     }

		     //потенциальное закрытие импульса по объемам
		     //две свечки подряд объем меньше 60% первой //или второй свечки импульса
                     //если объем свечки больше чем первой и направление свечки в другую сторону
		     if(!impulses[count_impulses].ind_closeimp && ind_enter > 0 &&
			( (i >= ind_impuls_st + 2 &&
			( (cndl.volume < cndl2->volume*0.5 && cndl1.volume < cndl2->volume*0.5) || cndl.volume < 3000)) ||
			(cndl.volume >= cndl2->volume && up && cndl.priceclose - 50 < cndl.priceopen) ||
			(cndl.volume >= cndl2->volume && !up && cndl.priceclose + 50 > cndl.priceopen) ))
		     {
			    impulses[count_impulses].ind_closeimp = i;
			    if(up) totalsum2 += cndl.priceclose - candles[CurTimeFrame][ind_enter].priceclose;
			    else   totalsum2 += candles[CurTimeFrame][ind_enter].priceclose - cndl.priceclose;
		     }

		     if(up &&
			(cndl.priceclose - cndl2->priceopen < d/2 ||                  //упали до половины первой свечки в импульсе
			(cndl.priceclose < candles[CurTimeFrame][i - 1].priceopen &&  //последняя свечка (цена закрытия) меньше предыдущей (цены открытия) и свечка больше равна 60 (закрытие - открытие)
			 cndl.priceclose - cndl.priceopen >= 60) ||
			 maxpriceclose - cndl.priceclose >= 200  ))                     //последняя свечка (цена закрытия) ушла на 60 или больше пунктов от максимального пика импульса
		     {
			    if(cndl.priceclose - cndl2->priceopen >= 300 || ind_enter)
			    {
			       impulses[count_impulses].ind_start = ind_impuls_st;
			       impulses[count_impulses].ind_end   = i;
			       impulses[count_impulses].ind_enter = ind_enter;
			       impulses[count_impulses].up        = up;
			       totalSum += cndl.priceclose - candles[CurTimeFrame][ind_enter].priceclose;
			       if(!impulses[count_impulses].ind_closeimp)
				      totalsum2 += cndl.priceclose - candles[CurTimeFrame][ind_enter].priceclose;
			       count_impulses++;
			    }
			    ind_impuls_st = -1;
		     }

		     if(!up && (cndl2->priceopen - cndl.priceclose < d/2 ||
			(candles[CurTimeFrame][i - 1].priceopen < cndl.priceclose &&
			 cndl.priceopen - cndl.priceclose >= 60) ||
			 (cndl.priceclose - minpriceclose >= 200 && minpriceclose)  ))
		     {
			    if(cndl2->priceopen - cndl.priceclose >= 300 || ind_enter)
			    {
			           impulses[count_impulses].ind_start = ind_impuls_st;
			           impulses[count_impulses].ind_end   = i;
			           impulses[count_impulses].ind_enter = ind_enter;
			           impulses[count_impulses].up        = up;
			           totalSum += candles[CurTimeFrame][ind_enter].priceclose - cndl.priceclose;
			           if(!impulses[count_impulses].ind_closeimp)
				          totalsum2 += candles[CurTimeFrame][ind_enter].priceclose - cndl.priceclose;
			           count_impulses++;
			        }
			        ind_impuls_st = -1;
		         }
		         //после импульса отмотаем на индекс назад, чтобы проверить этот же индекс
		         //на начало импульса в новой итерации
		         if(ind_impuls_st < 0) i--;
		         else
		         {
			         if(cndl.priceclose > maxpriceclose)
				        maxpriceclose = cndl.priceclose;
			         if(cndl.priceclose < minpriceclose || !minpriceclose)
			            minpriceclose = cndl.priceclose;
		         }
	        }
       }
       DrawAll();
       Itogo->Caption = "Итого: " + IntToStr((int)totalSum) + " (" + IntToStr((int)totalsum2) + ")";
       Marga->Caption = "Маржа: " + IntToStr((int)curMarga);
}
//---------------------------------------------------------------------------

void __fastcall TFMain::Button12Click(TObject *Sender)
{
       count_extremums = 0;
       ZeroMemory(extremums, sizeof(struct extremum)*10000);

       int period = 6;
       float mindiffextr = 0;
       TryStrToFloat(DiffForExtr->Text, mindiffextr);

       //минимумы и максимумы чередуются друг за другом
       //последний экстремум обновляется по мере поступления свечек, если это не другой экстремум
       //минимальная разница между экстремумами или их свечками, их сложные комбинации

       bool lastmin = false;
       bool lastmax = true;

       float curmin = 0;
       float curmax = 0;

       bool first = true;
       for(int i = 2; i < count_candles[CurTimeFrame]; i++)
       {
	      struct candlesdata cndl1  = candles[CurTimeFrame][i - 2];
	      struct candlesdata cndl2  = candles[CurTimeFrame][i - 1];
	      struct candlesdata cndl3  = candles[CurTimeFrame][i];

	      if( (PHigh_Low->Checked && cndl2.low < cndl1.low && cndl2.low <= cndl3.low) ||
		  (PClose->Checked    && cndl2.priceclose < cndl1.priceclose && cndl2.priceclose <= cndl3.priceclose) )
	      {
		     float curval = 0;
		     if(PHigh_Low->Checked) curval = cndl2.low;
		     if(PClose->Checked)    curval = cndl2.priceclose;

		     //если был максимум, то запоминаем новый минимум, если был минимум, то его обновляем
		     if(lastmax)
		     {
			     //минимум после максимума, если между ними разница не менее mindiffextr
			     if(fabs(curval /*curmin*/ - curmax) >= mindiffextr &&
			       (!Min_menshe_max->Checked || curval < curmax) )
			     {
			             extremums[count_extremums].index = i - 1;
			             extremums[count_extremums].val   = curval;
			             extremums[count_extremums].min   = true;
			             count_extremums++;
			             lastmin = true;
			             lastmax = false;
			             curmin = curval;
			     }
		     }
		     else
		     {
			     if(curval <= extremums[count_extremums - 1].val && count_extremums > 0)
			     {
				    extremums[count_extremums - 1].index = i - 1;
				    extremums[count_extremums - 1].val   = curval;
				    curmax = curval;
			     }
                     }
             }

	      if( (PHigh_Low->Checked && cndl2.high > cndl1.high && cndl2.high >= cndl3.high) ||
		  (PClose->Checked    && cndl2.priceclose > cndl1.priceclose && cndl2.priceclose >= cndl3.priceclose) )
	      {
		     float curval = 0;
		     if(PHigh_Low->Checked) curval = cndl2.high;
		     if(PClose->Checked)    curval = cndl2.priceclose;

		     //если был максимум, то запоминаем новый минимум, если был минимум, то его обновляем
		     if(lastmin)
		     {
			     //максимум после минимума, если между ними разница не менее mindiffextr
			     if(fabs(curmin - curval /*curmax*/) >= mindiffextr &&
			       (!Min_menshe_max->Checked || curmin < curval) )
			     {
				     extremums[count_extremums].index = i - 1;
				     extremums[count_extremums].val   = curval;
				     extremums[count_extremums].min   = false;
				     count_extremums++;
				     lastmin = false;
				     lastmax = true;
				     curmax = curval;
                             }
		     }
		     else
		     {
			     if(curval >= extremums[count_extremums - 1].val && count_extremums > 0)
			     {
				     extremums[count_extremums - 1].index = i - 1;
				     extremums[count_extremums - 1].val   = curval;
				     curmin = curval;
			     }
                     }
	      }
       }
       DrawAll();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::Button13Click(TObject *Sender)
{
       count_impulses = 0;
       DrawAll();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::HeikenAshiClick(TObject *Sender)
{
       DrawAll();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::N14Click(TObject *Sender)
{
       EvanNeiroSet->Show();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::ODSelectionChange(TObject *Sender)
{
	OD->FileName;
	int aa = 11;
}
//---------------------------------------------------------------------------
void __fastcall TFMain::ODShow(TObject *Sender)
{
	OD->FileName;
	int aa = 11;
}
//---------------------------------------------------------------------------
void __fastcall TFMain::DopGraphicPricesClick(TObject *Sender)
{
       DrawAll();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::FormMouseWheel(TObject *Sender, TShiftState Shift, int WheelDelta,
          TPoint &MousePos, bool &Handled)
{
        ScrollBar1->Position += -5*WheelDelta/120;
        Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::N15Click(TObject *Sender)
{
        FormHelp->Show();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::FormCl(TObject *Sender, TCloseAction &Action)
{
        Application->Terminate();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::FormClQuery(TObject *Sender, bool &CanClose)
{
        Application->Terminate();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::FormClDestroy(TObject *Sender)
{
        Application->Terminate();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::Button14Click(TObject *Sender)
{
        iframes[0].height = 400;
        DrawAll();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::Button15Click(TObject *Sender)
{
        iframes[0].height = ClientHeight - 60 - iframes[0].y - Simulation->Height;
        struct frames* fr_orders = SearchFrame(TYPE_ORDERS_1, "_orders_");
        if(fr_orders && fr_orders->visible)
        iframes[0].height -= fr_orders->height;
        DrawAll();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::Button16Click(TObject *Sender)
{
        iframes[0].height = 500;
        DrawAll();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::FormClose(TObject *Sender, TCloseAction &Action)
{
        struct frames* fr_orders = SearchFrame(TYPE_ORDERS_1, "_orders_");
        if(!fr_orders)
	     return;

        fr_orders->visible = false;
}
//---------------------------------------------------------------------------
void __fastcall TFMain::Button17Click(TObject *Sender)
{
        graphics_orders = !graphics_orders;
        curframe = &iframes[0];
        DrawAll();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::BeziersGrOrdersClick(TObject *Sender)
{
        bool s = ((TCheckBox*)Sender)->Checked;

        LinesGrOrders->OnClick   = NULL;
        BeziersGrOrders->OnClick = NULL;
        CurvesGrOrders->OnClick  = NULL;
        Sma7GrOrders->OnClick    = NULL;
        Sma11GrOrders->OnClick   = NULL;
        Sma24GrOrders->OnClick   = NULL;

        LinesGrOrders->Checked   = false;
        BeziersGrOrders->Checked = false;
        CurvesGrOrders->Checked  = false;
        Sma7GrOrders->Checked    = false;
        Sma11GrOrders->Checked   = false;
        Sma24GrOrders->Checked   = false;

        ((TCheckBox*)Sender)->Checked = s;

        LinesGrOrders->OnClick   = BeziersGrOrdersClick;
        BeziersGrOrders->OnClick = BeziersGrOrdersClick;
        CurvesGrOrders->OnClick  = BeziersGrOrdersClick;
        Sma7GrOrders->OnClick    = BeziersGrOrdersClick;
        Sma11GrOrders->OnClick   = BeziersGrOrdersClick;
        Sma24GrOrders->OnClick   = BeziersGrOrdersClick;

        Invalidate();
	    curframe = &iframes[0];
        DrawAll();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::Button18Click(TObject *Sender)
{
        spectr_fft = !spectr_fft;
        curframe = &iframes[0];
        DrawAll();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::SpectrBuyClick(TObject *Sender)
{
        curframe = &iframes[0];
        DrawAll();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::N315cit1Click(TObject *Sender)
{
	if(count_candles[3] <= 0 || !candles[3])
	{
		ShowMessage("Нет данных");
		return;
	}

	typedef struct candlesdata2
	{
	  float priceopen;
	  float priceclose;
	  float low;
	  float high;
	  __int64 timeopen;
	  __int64 volume;

	} candlesdata2;

	// Найдём первую свечку месяца четыре месяца назад
	int m = tmdmy_candles[3][count_candles[3] - 1].month;
	int ch = 0;
	int starti[5];
	starti[0] = count_candles[3] - 1;
	starti[1] = count_candles[3] - 1;
	starti[2] = count_candles[3] - 1;
	starti[3] = count_candles[3] - 1;
	starti[4] = count_candles[3] - 1;

	for(int i = count_candles[3] - 1; i >= 0; i--)
	{
		if(tmdmy_candles[3][i].month != m)
		{
		    ch++;
			m = tmdmy_candles[3][i].month;

			if(ch == 1)
				starti[3] = i + 1;

			if(ch == 2)
				starti[2] = i + 1;

			if(ch == 3)
				starti[1] = i + 1;

			if(ch == 4)
			{
				starti[0] = i + 1;
				break;
			}
		}
	}

	// Сохраним в файлы
	for(int i = 0; i < 4; i++)
	{
		AnsiString month = IntToStr(tmdmy_candles[3][starti[i]].month);
		if(month.Length() == 1)
                     month = "0" + month;
		AnsiString filename = "RI_3_" + month + "." + IntToStr(tmdmy_candles[3][starti[i]].year) + ".cit";
		int ct_data = starti[i + 1] - starti[i];
		struct candlesdata2* cd = (struct candlesdata2*)malloc(sizeof(struct candlesdata2)*ct_data);
		ZeroMemory(cd, sizeof(struct candlesdata2)*ct_data);
		for(int n = 0; n < ct_data; n++)
		{
			cd[n].priceopen  = candles[3][starti[i] + n].priceopen;
			cd[n].priceclose = candles[3][starti[i] + n].priceclose;
			cd[n].low      = candles[3][starti[i] + n].low;
			cd[n].high     = candles[3][starti[i] + n].high;
			cd[n].timeopen = candles[3][starti[i] + n].timeopen;
			cd[n].volume   = candles[3][starti[i] + n].volume;
		}

		FILE* fl = fopen(filename.c_str(), "wb");
		if(fl)
		{
			fwrite("cit001", 6, 1, fl);
			fwrite(&ct_data, sizeof(int), 1, fl);
			fwrite(cd, sizeof(struct candlesdata2)*ct_data, 1, fl);
			fclose(fl);
		}
	}
}
//---------------------------------------------------------------------------


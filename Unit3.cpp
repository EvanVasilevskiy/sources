//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit3.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSmartTest *SmartTest;
//---------------------------------------------------------------------------
__fastcall TSmartTest::TSmartTest(TComponent* Owner)
	: TForm(Owner)
{
    Application->OnMinimize = MinimizeProc;
    Left = Screen->Width/2 - Width/2;
    Top  = Screen->Height/2 - Height/2;

    //ZeroMemory(resvirag, sizeof(int)*4*7*15);
    ZeroMemory(resvirag, sizeof(bool*)*4);
    ZeroMemory(resvirag_hash, sizeof(int*)*4);
    ZeroMemory(resvirag_sub, sizeof(bool*)*4*50);
    ZeroMemory(resvirag_sub_hash, sizeof(int*)*4*50);

    for(int d = 0; d < 6; d++)
    {
        for(int h = 0; h < 15; h++)
	{
	    PervUslBuy[d][h]  = (struct viragenie*)malloc(sizeof(struct viragenie)*MAX_COUNT_PERV_USLOV);
	    VstrUslSell[d][h] = (struct viragenie*)malloc(sizeof(struct viragenie)*MAX_COUNT_VSTR_USLOV);
	    PervUslSell[d][h] = (struct viragenie*)malloc(sizeof(struct viragenie)*MAX_COUNT_PERV_USLOV);
	    VstrUslBuy[d][h]  = (struct viragenie*)malloc(sizeof(struct viragenie)*MAX_COUNT_VSTR_USLOV);
	}
    }
}
//---------------------------------------------------------------------------
int __fastcall TSmartTest::RaspoznavanieUslovii2(AnsiString text, struct viragenie* uslov)
{
   int count_uslov  = 0;
   if(text == "") 
      return 0;
   
   //условия разделяются запятыми
   while(1)
   {
       //делим выражение на услоыия
       int pos = text.AnsiPos(",");
       AnsiString usl = Trim(text);
       if(pos > 0)
       {
	   usl  = Trim(text.SubString(1, pos - 1));
	   text = text.SubString(pos + 1, text.Length() - pos);
       }
       if(usl == "" && pos <= 0) break;
       if(usl == "") continue;

       uslov[count_uslov] = *RaspoznavanieVirageniya2(usl);
       uslov[count_uslov].viragenie_sub_hash = uslov[count_uslov].CalcHash(usl);       // Для ускорения анализа. путем запоминания результата выражения для каждой свечки
       count_uslov++;

       if(pos <= 0) break;
   }                       

   return count_uslov;
}
//---------------------------------------------------------------------------
struct viragenie* __fastcall TSmartTest::RaspoznavanieVirageniya2(AnsiString text)
{
   struct viragenie* v = NULL, *v2 = NULL;
   if(text == "") return v;

   //ищем логические операции & и |
   int count_skb = 0;
   for(int i = 1; i <= text.Length(); i++)
   {
	if(text[i] == '(') count_skb++;
	if(text[i] == ')') count_skb--;
	if(count_skb == 0 && (text[i] == '&' || text[i] == '|'))
	{
	    v = CreateVirageniya(0);
	    v->last = NULL;
	    AnsiString subText = text.SubString(1, i - 1);
	    v->sub  = RaspoznavanieVirageniya2(subText);
	    v->viragenie_sub_hash = v->CalcHash(subText);

	    v2 = CreateVirageniya(text[i]);
	    v2->last = NULL;
	    subText = text.SubString(i + 1, text.Length() - i);
	    v2->sub  = RaspoznavanieVirageniya2(subText);
	    v2->viragenie_sub_hash = v->CalcHash(subText);
	    v->next = v2;
	    v->next->last = v;

	    return v;
	}
   }

   AnsiString vl        = "";
   AnsiString curparams = "";
   AnsiString curback   = "";
   AnsiString curcolor  = "";
   AnsiString subbackindexs  = "";
   int skb_kv_num = 0;

   char bef_oper = '\0';
   int sk        = 0;
   bool skb      = false;
   bool skb_kv   = false;
   bool skb_fig  = false;  //фиг. скобки для плавающего условия
   for(int i = 1; i <= text.Length(); i++)
   {
	  char ch = text[i];
	  char ch2 = 0;
	  if(i < text.Length()) ch2 = text[i + 1];
	  char ch3 = 0;
	  if(i > 1) ch3 = text[i - 1];
	  //если сложный знак, его обработали ранее и идем дальше
	  if((ch3 == '<' || ch3 == '>' || ch3 == '!') && ch == '=')
		 continue;

	  if(skb && ch == '(') sk++;
	  if(skb && ch == ')') sk--;
	  if(sk > 0)
	  {
		  if(sk == 1)
		  {
			   if(ch == '{') skb_fig = true;
			   if(ch == '}') skb_fig = false;
			   if(skb_fig)
			   {
				   if(ch != '{')
				   {
					  subbackindexs += (AnsiString)ch;
				   }
			   }
			   else
				  vl += (AnsiString)ch;
		  }
		  else vl += (AnsiString)ch;

	  }
	  else
	  {
		 if(skb)
		 {
			vl = Trim(vl);
			subbackindexs = Trim(subbackindexs);
			if(vl != "")
			{
				struct viragenie* vir = CreateVirageniya(bef_oper);
				vir->sub = RaspoznavanieVirageniya2(vl);
				vir->viragenie_sub_hash = vir->CalcHash(vl);
				vir->SetSubBackIndexs(subbackindexs);
				subbackindexs = "";

				//если операция:* / ^ , то группируем с предыдущим, если он был
				if(v2 && (bef_oper == '*' || bef_oper == '/' || bef_oper == '^') )
				{
					struct viragenie* v3 = v2;
					v2 = CreateVirageniya(v3->before_operation);
					v2->last = v3->last;
					v3->before_operation = '\0';
					v2->sub = v3;
					v3->next = vir;
					vir->last = v3;

					if(v3 == v) v = v2;
					else
					{
					    if(v3->last)
						v3->last->next = v2;
				        }
					v3->last = NULL;
				}
				else
				{
				   if(!v) v = vir;
				   else
				   {
					  v2->next = vir;
					  vir->last = v2;
				   }
				   v2 = vir;
				}

				skb = false;
				vl = "";
				bef_oper = '\0';
			}
		 }
		 else
		 {

			if((ch != '+' && ch != '-' && ch != '*' && ch != '/' && ch != '^' &&
			   ch != '&' && ch != '|' && ch != '(' && ch != ')' && ch != '=' &&
			   ch != '<' && ch != '>' && ch != '!') || skb_kv)
			{
			   //если квадратная скобка, значит еще инструкции, в них могут быть + -
			   if(ch == '[') { skb_kv = true; skb_kv_num++; }
			   if(ch == ']') skb_kv = false;
			   if(!skb_kv && !skb_fig)
			   {
				  if(ch != ']' && ch != '}') vl += (AnsiString)ch;
			   }
			   if(skb_kv)
			   {
				   if(ch != '[')
				   {
					  if(skb_kv_num == 1) curparams += (AnsiString)ch;
					  if(skb_kv_num == 2) curback   += (AnsiString)ch;
					  if(skb_kv_num == 3) curcolor  += (AnsiString)ch;
				   }
			   }
			}
			else
			{
			   vl = Trim(vl);
			   curparams     = Trim(curparams);
			   curback       = Trim(curback);
			   curcolor      = Trim(curcolor);
			   if(vl != "")
			   {
				  struct viragenie* vir = CreateVirageniya(bef_oper);
				  vir->AddVal(vl);
				  vir->AddParams(curparams);
				  vir->SetBack(curback);
				  vir->SetColor(curcolor);

				  if(!vir->val)
					 int aa = 11;

				  //если операция:* / ^, то группируем с предыдущим, если он был
				  if(v2 && (bef_oper == '*' || bef_oper == '/' || bef_oper == '^') )
				  {
					 struct viragenie* v3 = v2;
					 v2 = CreateVirageniya(v3->before_operation);
					 v2->last = v3->last;
					 v3->before_operation = '\0';
					 v2->sub = v3;
					 v3->next = vir;
					 vir->last = v3;

					 if(v3 == v) v = v2;
					 else
					 {
						if(v3->last)
						   v3->last->next = v2;
					 }
					 v3->last = NULL;
				  }
				  else
				  {
					 if(!v) v = vir;
					 else
					 {
						v2->next = vir;
						vir->last = v2;
					 }
					 v2 = vir;
				  }

				  vl        = "";
				  curparams = "";
				  curback   = "";
				  curcolor  = "";
				  skb_kv_num = 0;
			   }
			   if(ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^' ||
				  ch == '&' || ch == '|' || ch == '<' || ch == '>' || ch == '=' ||
				  ch == '!')
			   {
				  bef_oper = ch;
				  if(ch2 == '=')
				  {
					  if(ch == '<') bef_oper = 1;
					  if(ch == '>') bef_oper = 2;
					  if(ch == '!') bef_oper = 3;
                                  }

				   //если операция сравнения, то группируем всё что до отдельно и после
				   //отдельно и вернем выражение
				   if(v && (bef_oper == '<' || bef_oper == '>' || bef_oper == '=' ||
					    bef_oper == '&' || bef_oper == '|' ||
							bef_oper == 1 || bef_oper == 2 || bef_oper == 3))
				   {
					struct viragenie* v3 = v;
					v = CreateVirageniya(0);
					v->last = NULL;
					v->sub  = v3;
					int ii = i;
					if(bef_oper == 1 || bef_oper == 2 || bef_oper == 3)
					ii ++;
					v3 = CreateVirageniya(bef_oper);
					AnsiString subText = text.SubString(ii + 1, text.Length() - ii);
					v3->sub = RaspoznavanieVirageniya2(subText);
					v3->viragenie_sub_hash = v3->CalcHash(subText);
					v->next = v3;
					v->next->last = v;

					return v;
				   }
			   }

			   if(ch == '(')
			   {
				   sk = 1;
				   skb = true;
			   }
		   }
	    }

       }
   }

   vl = Trim(vl);
   curparams = Trim(curparams);
   curback   = Trim(curback);
   curcolor  = Trim(curcolor);
   if(vl != "")
   {
	  struct viragenie* vir = CreateVirageniya(bef_oper);
	  vir->AddVal(vl);
	  vir->AddParams(curparams);
	  vir->SetBack(curback);
	  vir->SetColor(curcolor);
	  if(!vir->val)
		 int aa = 11;

	  //если операция:* / ^ , то группируем с предыдущим, если он был
	  if(v2 && (bef_oper == '*' || bef_oper == '/' || bef_oper == '^') )
	  {
		 struct viragenie* v3 = v2;
		 v2 = CreateVirageniya(v3->before_operation);
		 v2->last = v3->last;
		 v3->before_operation = '\0';
		 v2->sub = v3;
		 v3->next = vir;
		 vir->last = v3;

		 if(v3 == v) v = v2;
		 else
		 {
			if(v3->last)
			   v3->last->next = v2;
		 }
		 v3->last = NULL;
	  }
	  else
	  {

		  //если операция сравнения, то группируем всё что до отдельно и после
		  //отдельно и вернем выражение
		  if(v && (bef_oper == '<' || bef_oper == '>' || bef_oper == '=' ||
					bef_oper == '&' || bef_oper == '|' ||
					bef_oper == 1 || bef_oper == 2 || bef_oper == 3))
		  {
			  struct viragenie* v3 = v;
			  v = CreateVirageniya(0);
			  v->last = NULL;
			  v->sub  = v3;

			  v->next = vir;
			  v->next->last = v;
			  v->next->before_operation = bef_oper;

			  return v;
		  }

		  if(!v) v = vir;
		  else
		  {
			  v2->next = vir;
			  vir->last = v2;
		  }
		  v2 = vir;
	  }
   }

   return v;
}
//---------------------------------------------------------------------------
struct viragenie* __fastcall TSmartTest::CreateVirageniya(char operation)
{
   struct viragenie* vir = (struct viragenie*)malloc(sizeof(struct viragenie));
   ZeroMemory(vir, sizeof(struct viragenie));
   vir->before_operation = operation;
   return vir;
}
//---------------------------------------------------------------------------
void __fastcall TSmartTest::ParserConstants(AnsiString ConstVals)
{
   count_const = 0;

   AnsiString vl = "";
   for(int i = 1; i <= ConstVals.Length(); i++)
   {
	   char ch = ConstVals[i];
	   if(ch != ',') vl += (AnsiString)ch;
	   else
	   {
		   vl = Trim(vl);
		   if(vl != "")
		   {
			  //разделяем на две части, переменная=значение
			  int pos = vl.AnsiPos("=");
			  if(pos > 0)
			  {
				 const_val[count_const] = vl.SubString(pos + 1, vl.Length() - pos);

				 // Проверим на относительное число
				 if(const_val[count_const].Length() > 0 && const_val[count_const][1] == '%')
				 {
					 const_otnosit[count_const] = true;
					 const_val[count_const] = const_val[count_const].SubString(2, const_val[count_const].Length() - 1);
				 }
				 else
                                    const_otnosit[count_const] = false;

				 int resval = 0;
				 if(!TryStrToInt(const_val[count_const], resval))
				    resval = 0;
				 iconst_val[count_const] = resval;
				 float fresval = 0;
				 if(!TryStrToFloat(iFloat(const_val[count_const]), fresval))
					fresval = 0;
				 fconst_val[count_const] = fresval;

				 vl = vl.SubString(1, pos - 1);
			  }

			  const_per[count_const] = vl;
			  count_const ++;
			  vl = "";
		   }
	   }
   }
   vl = Trim(vl);
   if(vl != "")
   {
	  //разделяем на две части, переменная=значение
	  int pos = vl.AnsiPos("=");
	  if(pos > 0)
	  {
		 const_val[count_const] = vl.SubString(pos + 1, vl.Length() - pos);
		 int resval = 0;
		 if(!TryStrToInt(const_val[count_const], resval))
		    resval = 0;
		 iconst_val[count_const] = resval;
		 float fresval = 0;
		 if(!TryStrToFloat(iFloat(const_val[count_const]), fresval))
		    fresval = 0;
		 fconst_val[count_const] = fresval;

		 vl = vl.SubString(1, pos - 1);
	  }

	  const_per[count_const] = vl;
	  count_const ++;
   }
}
//---------------------------------------------------------------------------
AnsiString __fastcall TSmartTest::iFloat(AnsiString str)
{
   int pos = str.AnsiPos(".");
   if(pos > 0) str = str.SubString(1, pos - 1) + "," +
					 str.SubString(pos + 1, str.Length() - pos );
   return str;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TSmartTest::DelComments(AnsiString Text)
{
	if(Text == "")
	  return Text;

	// Удалим комментарии /* */
	while(1)
	{
		int pos1 = Text.AnsiPos("/*");
		int pos2 = Text.AnsiPos("*/");

		if(pos1 > 0 && pos2 <= 0)
		   pos2 = Text.Length();

		if(pos1 <= 0 || pos2 <= 0 || pos2 - pos1 < 2)
		   return Text;

		Text = Text.SubString(1, pos1 - 1) + Text.SubString(pos2 + 2, Text.Length() - pos2 - 1);
	}

	return Text;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TSmartTest::UsePsevdonimy(AnsiString Text, AnsiString* psevd,
												int count_psevd)
{
   if(count_psevd <= 0 || !psevd) return Text;

   // Добавим замену цветов
   AnsiString usl_cols[13] = {"green=color[g]", "red=color[g]", "blue=color[b]", "yellow=color[y]", "orange=color[or]",
							  "teal=color[t]", "maroon=color[m]", "olive=color[o]", "navy=color[n]", "purple=color[p]",
							  "fuchsia=color[f]", "lime=color[l]", "aqua=color[a]"};
   for(int i = 0; i < 13; i++)
   {
	   psevd[count_psevd + i] = usl_cols[i];
   }
   count_psevd += 13;

   int ch = 0;
   for(int i = 0; i < count_psevd; i++)
   {
	   AnsiString name = "";
	   AnsiString val  = "";
	   int posname = psevd[i].AnsiPos("=");
	   if(posname <= 0) continue;
	   name = Trim(psevd[i].SubString(1, posname - 1));
	   val  = Trim(psevd[i].SubString(posname + 1, psevd[i].Length() - posname));
	   while(1)
	   {
		  int pos = Text.AnsiPos(name);
		  if(pos > 0)
			 Text = Text.SubString(1, pos - 1) + val +
					Text.SubString(pos + name.Length(), Text.Length() - pos - name.Length() + 1);
		  else break;
		  ch++;
	      if(ch > 5000) break;
	   }
	   if(ch > 5000) break;
   }

   return Text;
}
//---------------------------------------------------------------------------

//удален код

//---------------------------------------------------------------------------
void __fastcall TSmartTest::ButtonMassAnalizeClick(TObject *Sender)
{
   __int64 totaltime = GetTickCount();
   ct_Indic = 0;
   
   for(int d = 0; d < 6; d++)
   {
       for(int h = 0; h < 15; h++)
	   {
		   ZeroMemory(PervUslBuy[d][h],  sizeof(struct viragenie)*MAX_COUNT_PERV_USLOV);
		   ZeroMemory(PervUslSell[d][h],  sizeof(struct viragenie)*MAX_COUNT_PERV_USLOV);
		   ZeroMemory(VstrUslBuy[d][h],  sizeof(struct viragenie)*MAX_COUNT_VSTR_USLOV);
		   ZeroMemory(VstrUslSell[d][h],  sizeof(struct viragenie)*MAX_COUNT_VSTR_USLOV);
       }
   }

   //заменим псевдонимы на их функции для 5 дней и общие
   AnsiString psevd[6][15][100];
   int count_psevd[6][15]; // = 0;
   ZeroMemory(count_psevd, sizeof(int)*6*15);
   //Explode(psevd, count_psevd, Psevdonimy->Text, ",");

   ZeroMemory(virag_hash, sizeof(int)*4*7*15);
   struct viragenie tv;

   // Максимальное кол-во подусловий
   int ct_subvirag[4];
   ZeroMemory(ct_subvirag, sizeof(int)*4);

   // Распознаем условия и выражения общие и для 5 дней                     
   for(int i = 0; i < 6; i++)
   {
	   for(int h = 0; h < 15; h++)
	   {              PervUslBuy[i][h];
		   Explode(psevd[i][h], count_psevd[i][h], PsevdonimyTextDays[i][h], ",");

		   AnsiString pub = Trim(DelComments(UsePsevdonimy(UslPervBuyTextDays[i][h],  psevd[i][h], count_psevd[i][h])));
		   AnsiString pus = Trim(DelComments(UsePsevdonimy(UslPervSellTextDays[i][h], psevd[i][h], count_psevd[i][h])));
		   AnsiString vub = Trim(DelComments(UsePsevdonimy(UslVstrBuyTextDays[i][h],  psevd[i][h], count_psevd[i][h])));
		   AnsiString vus = Trim(DelComments(UsePsevdonimy(UslVstrSellTextDays[i][h], psevd[i][h], count_psevd[i][h])));

		   count_PervUslBuy[i][h]  = RaspoznavanieUslovii2(pub, PervUslBuy[i][h]);
		   count_PervUslSell[i][h] = RaspoznavanieUslovii2(pus, PervUslSell[i][h]);
		   count_VstrUslBuy[i][h]  = RaspoznavanieUslovii2(vub, VstrUslBuy[i][h]);
		   count_VstrUslSell[i][h] = RaspoznavanieUslovii2(vus, VstrUslSell[i][h]);

           virag_hash[0][i][h] = tv.CalcHash(pub);
		   virag_hash[1][i][h] = tv.CalcHash(pus);
		   virag_hash[2][i][h] = tv.CalcHash(vub);
		   virag_hash[3][i][h] = tv.CalcHash(vus);

		   if(count_PervUslBuy[i][h]  > ct_subvirag[0]) ct_subvirag[0] = count_PervUslBuy[i][h];
		   if(count_PervUslSell[i][h] > ct_subvirag[1]) ct_subvirag[1] = count_PervUslSell[i][h];
		   if(count_VstrUslBuy[i][h]  > ct_subvirag[2]) ct_subvirag[2] = count_VstrUslBuy[i][h];
		   if(count_VstrUslSell[i][h] > ct_subvirag[3]) ct_subvirag[3] = count_VstrUslSell[i][h];
	   }
   }
   //константы
   ParserConstants(ConstVals->Text);

   CurCountCandles = FMain->count_candles;                    
   for(int i = 0; i < 10; i++)
	  CurCandles[i] = (FMain->candles[i]);                  
   for(int i = 0; i < 10; i++)
	  cur_tmdmy_candles[i] = (FMain->tmdmy_candles[i]);
   //ZeroMemory(curIndex, sizeof(int)*10);

   //таймфрейм торговли
   int maintf  = fGetPerConstOrTable("TF");

   // Запомненные результаты выражений для индексов по хешу
   try
   {
	   for(int t = 0; t < 4; t++)
	   {
		   if(resvirag[t] && (!OptimizAnalizPoUslov->Checked || maintf != resvirag_tf))
		   {
			   free(resvirag[t]);
			   free(resvirag_hash[t]);
			   resvirag[t]      = NULL;
			   resvirag_hash[t] = NULL;
		   }

		   try
		   {
			   if(OptimizAnalizPoUslov->Checked && !resvirag[t])
			   {
				   resvirag[t]      = (bool*)malloc(sizeof(bool)*(CurCountCandles[maintf]));
				   resvirag_hash[t] = (int*)malloc(sizeof(int)*(CurCountCandles[maintf]));
				   if(resvirag[t])
					  ZeroMemory(resvirag[t], sizeof(bool)*(CurCountCandles[maintf]));
				   else
				   {
					   ShowMessage("Недостаточно памяти");
					   return;
				   }
			   }
		   }
		   catch(...)
		   {
			   int aa = 11;
		   }

		   for(int n = 0; n < MAX_COUNT_PERV_USLOV && n < ct_subvirag[t]; n++)
		   {
			   if(resvirag_sub[t][n] && (!OptimizAnalizPoUslov->Checked || maintf != resvirag_tf))
			   {
				   free(resvirag_sub[t][n]);
				   free(resvirag_sub_hash[t][n]);
				   resvirag_sub[t][n]      = NULL;
				   resvirag_sub_hash[t][n] = NULL;
			   }

			   try
			   {
				   if(OptimizAnalizPoUslov->Checked && !resvirag_sub[t][n])
				   {
					   resvirag_sub[t][n]      = (bool*)malloc(sizeof(bool)*(CurCountCandles[maintf]));
					   resvirag_sub_hash[t][n] = (int*)malloc(sizeof(int)*(CurCountCandles[maintf]));

					   if(resvirag_sub[t][n])
						  ZeroMemory(resvirag_sub[t][n], sizeof(bool)*(CurCountCandles[maintf]));
					   if(resvirag_sub_hash[t][n])
						  ZeroMemory(resvirag_sub_hash[t][n], sizeof(int)*(CurCountCandles[maintf]));
					   else
					   {
						   ShowMessage("Недостаточно памяти");
						   return;
					   }
				   }
			   }
			   catch(...)
			   {
				   int aa = 11;
			   }
		   }
	   }

	   resvirag_tf = maintf;
   }
   catch(...)
   {
	   int aa = 11;
   }

   //сделки
   count_orders     = 0;
   taimframe_orders = 0;
   ZeroMemory(orders, sizeof(struct iorders)*MAX_COUNT_ORDERS);


   curAllDataInstruments->Clear();
   curAllDataInstruments = NULL;

   FMain->Simulation->Position = 0;

   // Получим список годов и индексы старта и конца года
   int years[64];
   int ind_start_years[64];
   int ind_end_years[64];
   int ct_years = 0;
   int curyear = 0;
   for(int i = 0; i < CurCountCandles[maintf]; i++)
   {
	   int year = GetYear(CurCandles[maintf][i].timeopen);
	   if(year != curyear)
	   {
		   years[ct_years] = year;
		   ind_start_years[ct_years] = i;
		   curyear = year;
           ct_years++;
	   }

	   ind_end_years[ct_years - 1] = i;
   }

   ct_years = 2;
   int ct_ind = CurCountCandles[maintf]/ct_years;
   for(int i = 0; i < ct_years; i++)
   {
	   ind_start_years[i] = i*ct_ind;
	   ind_end_years[i]   = i*ct_ind + ct_ind - 1;
   }
   ind_end_years[ct_years - 1] = CurCountCandles[maintf] - 1;

   if(!AnalizeThreads->Checked)
   {
	   ind_end_years[0] = CurCountCandles[maintf] - 1; 
	   ct_years = 1;
   }

   // Создадим рабочее пространство для анализа для каждого года
   void* data_analize[64][11];
   HANDLE ihThreads[64];

   for(int y = 0; y < ct_years; y++)
   {
	   // Прибыль по часам, дням, месяцам, годам
	   float* avg_marga_days_week  = (float*)malloc(sizeof(float)*7);
	   float* avg_marga_hours_day  = (float*)malloc(sizeof(float)*24);
	   float* avg_marga_hours_day_perv = (float*)malloc(sizeof(float)*24);
	   float* avg_marga_month_year = (float*)malloc(sizeof(float)*12);
	   float* avg_marga_year       = (float*)malloc(sizeof(float)*2100);
	   float* avg_marga_days_month = (float*)malloc(sizeof(float)*31);
	   float* marga_all_days       = (float*)malloc(sizeof(float)*CurCountCandles[7]);
	   float* marga_all_days_sum   = (float*)malloc(sizeof(float)*CurCountCandles[7]);

	   if(!avg_marga_year || !marga_all_days || !marga_all_days_sum)
	   {
		   ShowMessage("Недостаточно памяти");
		   return;
	   }

	   ZeroMemory(avg_marga_days_week, sizeof(float)*7);
	   ZeroMemory(avg_marga_hours_day, sizeof(float)*24);
	   ZeroMemory(avg_marga_hours_day_perv, sizeof(float)*24);
	   ZeroMemory(avg_marga_month_year, sizeof(float)*12);
	   ZeroMemory(avg_marga_year,       sizeof(float)*2100);
	   ZeroMemory(avg_marga_days_month, sizeof(float)*31);
	   ZeroMemory(marga_all_days,       sizeof(float)*CurCountCandles[7]);
	   ZeroMemory(marga_all_days_sum,   sizeof(float)*CurCountCandles[7]);

	   // Ставим минус, нужно для того, чтобы правильно определить первый год для гистограммы
	   for(int i = 0; i < 2100; i++)
		  avg_marga_year[i] = -1000000;
	   for(int i = 0; i < CurCountCandles[7]; i++)
	   {
		   marga_all_days[i]     = -1000000;
		   marga_all_days_sum[i] = -1000000;
	   }

	   struct result_analize* ra = (struct result_analize*)malloc(sizeof(struct result_analize));
	   if(ra)
	      ZeroMemory(ra, sizeof(struct result_analize));

	   if(!ra)
	   {
		   ShowMessage("Недостаточно памяти");
		   return;
	   }

	   data_analize[y][0] = avg_marga_days_week;
	   data_analize[y][1] = avg_marga_hours_day;
	   data_analize[y][2] = avg_marga_hours_day_perv;
	   data_analize[y][3] = avg_marga_month_year;
	   data_analize[y][4] = avg_marga_year;
	   data_analize[y][5] = avg_marga_days_month;
	   data_analize[y][6] = marga_all_days;
	   data_analize[y][7] = marga_all_days_sum;
	   data_analize[y][8] = ra;
	   data_analize[y][9] = &ind_start_years[y];
	   data_analize[y][10] = &ind_end_years[y];

	   try
	   {
		   // Не прошли первый цикл - для создания всех индикаторов
		   wasstarti = false;
		   if(ct_years > 1)
		   {
			   // Создание и запуск потоков
			   ihThreads[y] = CreateThread(NULL, 0, &AnalizeThread, data_analize[y], /*CREATE_SUSPENDED*/ 0, NULL);
			   if(ihThreads[y] == NULL)
			   {
					ShowMessage("Анализ не успешен. Потоки не созданы\r\n");
			   }
		   }
		   else
		   {
			   AnalizeThread(data_analize[y]);
           }
		   int s = 0;
		   if(y == 0 && ct_years > 1)
			  for(s = 0; s < 10; s++)
			  {
				  if(wasstarti)
                     break;
				  Sleep(200);
			  }
	   }
	   catch(...)
	   {

	   }
   }

   Application->ProcessMessages();

   // Ждём завершения всех потоков
   WaitForMultipleObjects(ct_years, ihThreads, TRUE, INFINITE);

   totaltime = GetTickCount() - totaltime;
   TotalTimeLabel->Caption = FloatToStr(RoundTo((float)totaltime/1000.0, -2)) + " сек.";  ct_Indic;

   float* avg_marga_days_week      = (float*)data_analize[0][0];
   float* avg_marga_hours_day      = (float*)data_analize[0][1];
   float* avg_marga_hours_day_perv = (float*)data_analize[0][2];
   float* avg_marga_month_year     = (float*)data_analize[0][3];
   float* avg_marga_year           = (float*)data_analize[0][4];
   float* avg_marga_days_month     = (float*)data_analize[0][5];
   float* marga_all_days           = (float*)data_analize[0][6];
   float* marga_all_days_sum       = (float*)data_analize[0][7];
   struct result_analize* ra       = (struct result_analize*)data_analize[0][8];

   // Приведём результаты к нужному виду
   int chd    = 0;
   int endsumd = 0;
   int lastsumd = 0;
   for(int i = 0; i < CurCountCandles[7]; i++)
   {
		if(marga_all_days[i] > -1000000)
		{
			chd++;
			lastsumd = marga_all_days_sum[i];
		}
		else
		   marga_all_days[i] = 0;

		if(marga_all_days_sum[i] <= -1000000)
		   marga_all_days_sum[i] = 0;
   }

   for(int y = 1; y < ct_years; y++)
   {
	   for(int i = 0; i < 7; i++)
		  avg_marga_days_week[i] += ((float*)data_analize[y][0])[i];
	   for(int i = 0; i < 24; i++)
	   {
		   avg_marga_hours_day[i]      += ((float*)data_analize[y][1])[i];
		   avg_marga_hours_day_perv[i] += ((float*)data_analize[y][2])[i];
	   }
	   for(int i = 0; i < 12; i++)
		  avg_marga_month_year[i] += ((float*)data_analize[y][3])[i];

	   for(int i = 0; i < 2100; i++)
		  if(((float*)data_analize[y][4])[i] > -1000000)
			 avg_marga_year[i] = ((float*)data_analize[y][4])[i];

	   for(int i = 0; i < 31; i++)
		  avg_marga_days_month[i] += ((float*)data_analize[y][5])[i];

	   endsumd = lastsumd;
	   for(int i = 0; i < CurCountCandles[7]; i++)
	   {
		   if(((float*)data_analize[y][6])[i] > -1000000)
		   {
			   marga_all_days[chd]     = ((float*)data_analize[y][6])[i];
			   marga_all_days_sum[chd] = endsumd + ((float*)data_analize[y][7])[i];
			   chd++;
			   lastsumd = endsumd + ((float*)data_analize[y][7])[i];
		   }
	   }

	   ra->Sum((struct result_analize*)data_analize[y][8]);
   }

   for(int i = 0; i < 7; i++)
	  avg_marga_days_week[i] /= ct_years;
   for(int i = 0; i < 24; i++)
   {
		avg_marga_hours_day[i]      /= ct_years;
		avg_marga_hours_day_perv[i] /= ct_years;
   }
   for(int i = 0; i < 12; i++)
	  avg_marga_month_year[i] /= ct_years;
   for(int i = 0; i < 31; i++)
	  avg_marga_days_month[i] /= ct_years;

   ra->Delit(ct_years);
   ShowResText(ra);

   // Сделки
   count_orders = 0;
   for(int y = 0; y < ct_years; y++)
   {
	   struct result_analize* ra = (struct result_analize*)data_analize[y][8];
	   for(int i = 0; i < ra->count_orders; i++)
	   {
		   orders[count_orders] = ra->orders[i];
           count_orders++;
	   }
   }

   // Освободим много памяти
   for(int y = 1; y < ct_years; y++)
   {
	   for(int i = 0; i <= 8; i++)
		  if(data_analize[y][i])
			 free(data_analize[y][i]);
   }

   struct frames* frs[10];
   struct frames* fr7 = FMain->SearchOrAddFrame(TYPE_ORDERS_1, "_orders_");               frs[0] = fr7;
   struct frames* fr1 = FMain->SearchOrAddFrame(TYPE_HISTOGRAM_1, "_hist_daysofweek_");   frs[1] = fr1;
   struct frames* fr2 = FMain->SearchOrAddFrame(TYPE_HISTOGRAM_1, "_hist_perv");          frs[2] = fr2;
   struct frames* fr8 = FMain->SearchOrAddFrame(TYPE_HISTOGRAM_1, "_hist_vstr");          frs[3] = fr8;
   struct frames* fr3 = FMain->SearchOrAddFrame(TYPE_HISTOGRAM_1, "_hist_monthes");       frs[4] = fr3;
   struct frames* fr4 = FMain->SearchOrAddFrame(TYPE_HISTOGRAM_1, "_hist_daysofmonth");   frs[5] = fr4;
   struct frames* fr9 = FMain->SearchOrAddFrame(TYPE_HISTOGRAM_1, "_hist_years");         frs[6] = fr9;
   struct frames* fr5 = FMain->SearchOrAddFrame(TYPE_HISTOGRAM_2, "_hist_ordersdays");    frs[7] = fr5;
   struct frames* fr6 = FMain->SearchOrAddFrame(TYPE_HISTOGRAM_2, "_hist_ordersdayssum"); frs[8] = fr6;
   struct frames* fr10 = FMain->SearchOrAddFrame(TYPE_HISTOGRAM_1, "_hist_monthes_years"); frs[9] = fr10;

   // Освободим много памяти
   for(int i = 1 ; i < 10; i++)
	   if(frs[i] && i != 6)
	   {
		   if(frs[i]->data)
		   {
			   if(i == 2 || i == 3)
				  free((float*)frs[i]->data - 5);
			   else
				  free(frs[i]->data);
		   }
		   if(frs[i]->multidata) free(frs[i]->multidata);
		   frs[i]->data      = NULL;
		   frs[i]->multidata = NULL;
	   }

   for(int i = 0 ; i < 10; i++)
      frs[i]->visible = true;

   //удален код


   struct all_data_instruments* adi;                    
   adi = curAllDataInstruments->AddIndicator(fr7, "Сделки", 0, "", NULL,  0,  "");
   if(!curAllDataInstruments) curAllDataInstruments = adi;
   /*adi = */curAllDataInstruments->AddIndicator(fr1, "Распределение по дням недели", 0, "", avg_marga_days_week,  7,  "пн., вт., ср., чт., пт., сб., вс");
   curAllDataInstruments->AddIndicator(fr2, "Распределение по часам, первичные", 0,      "", avg_marga_hours_day_perv + 5,  24 - 5, /*"01, 02, 03, 04, 05,*/ "06, 07, 08, 09, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24");
   curAllDataInstruments->AddIndicator(fr8, "Распределение по часам, встречные", 0,      "", avg_marga_hours_day + 5,  24 - 5, /*"01, 02, 03, 04, 05,*/ "06, 07, 08, 09, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24");
   curAllDataInstruments->AddIndicator(fr3, "Распределение по месяцам", 0,     "", avg_marga_month_year, 12, "янв., фев., март, апр., май, июнь, июль, авг., сен., окт., ноя., дек.");
   curAllDataInstruments->AddIndicator(fr4, "Распределение по дням месяца", 0, "", avg_marga_days_month, 31, "01, 02, 03, 04, 05, 06, 07, 08, 09, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31");
   curAllDataInstruments->AddIndicator(fr5, "Распределение сделок по дням", 0,        "", marga_all_days,       CurCountCandles[7], "");
   curAllDataInstruments->AddIndicator(fr6, "Общая прибыль по дням", 0,        "", marga_all_days_sum,   CurCountCandles[7], "");
   curAllDataInstruments->AddIndicator(fr9, "Распределение по годам", 0,       "", start_marga_year, count_marga_year, str_years);
   curAllDataInstruments->AddIndicator(fr10, "Распределение по месяцам всех лет", 0, "", ra->GetMonthesYears(), ra->ct_years*13 - 1, ra->GetMonthesYearsStr());

   FMain->DrawAll();

   free(data_analize[0][8]);
}
//---------------------------------------------------------------------------
AnsiString __fastcall TSmartTest::GetUslRes(struct viragenie* uslov, int count_uslov)
{
   AnsiString zn = "";
   if(count_uslov)
   {
	  for(int u = 0; u < count_uslov; u++)
	  {
		 AnsiString zn1 = "";
		 bool estusl = false;
		 GetUslRes2(&(uslov[u]), zn1, estusl);
		 if(estusl)
		    zn += zn1;
	  }
   }
   return zn;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TSmartTest::GetUslRes2(struct viragenie* vir, AnsiString& izn,
											 bool& estusl)
{
   if(!vir) return 0;
   struct viragenie* vir2 = vir;
   AnsiString zn = "";

   while(1)
   {
		 char bo = vir->before_operation;

		 if(bo == '+' || bo == '-' || bo == '<' || bo == '>' || bo == '*' ||
			bo == '/' || bo == '&' || bo == '|' || bo == '=' || bo == '^' )
			zn += " " + (AnsiString)bo + " ";
		 if(bo == '<' || bo == '>' || bo == 1 ||
			bo == 2 || bo == 3 ||  bo == '=' )
			estusl = true;

		 if(bo == 1) zn += "<= ";
		 if(bo == 2) zn += "zn >= ";
		 if(bo == 3) zn += "zn != ";

	  if(vir->sub)
	  {
		  bool estusl2 = false;
		  AnsiString val = GetUslRes2(vir->sub, izn, estusl2);
		  zn += val;
		  if(estusl2)
		  izn += IntToStr(vir->countsubtrue) +
				 "   " + IntToStr(vir->countsubexec) + "   " + val + "\r\n";
		  if(estusl2) estusl = estusl2;
	  }
	  else
	  {

		 zn += (AnsiString)vir->val;
		 if(vir->params)
		 {
			zn += "[" +(AnsiString)vir->params + "]";
			zn += "[" + IntToStr(vir->back) + "]";
         }
	  }

		 if(vir->next) vir = vir->next;
		 else break;
   }
   return zn;
}
//---------------------------------------------------------------------------
bool __fastcall  TSmartTest::CheckExUsloviyaHourDayOfWeek(struct viragenie* uslov[6][15] /*uslov[6][15][MAX_COUNT_USLOV]*/,
		 int count_uslov[6][15], int num_virag_hash, int hour, int dayofweek, int* curIndex, struct techdata_order* to)
{
	if(dayofweek <= 0 || dayofweek >= 6)
	   dayofweek = 0;

	if(hour <= 0 || hour >= 24)
	   hour = 0;

	if(count_uslov[dayofweek][hour] <= 0)
	   hour = 0;

	if(count_uslov[dayofweek][hour] <= 0)
	   dayofweek = 0;

	if(count_uslov[dayofweek][hour] <= 0 || dayofweek == 5)
	   int aa = 11;

	if(!OptimizAnalizPoUslov->Checked)
	   return CheckExUsloviya(uslov[dayofweek][hour], count_uslov[dayofweek][hour], curIndex, to, num_virag_hash, hour, dayofweek);

	int ind = curIndex[resvirag_tf];
	try
	{
		if(ind >= 0 && resvirag_hash[num_virag_hash][ind] == virag_hash[num_virag_hash][dayofweek][hour])
		   return resvirag[num_virag_hash][ind];
	}
	catch(...)
	{
		int aa = 11;
	}

	bool resusl = CheckExUsloviya(uslov[dayofweek][hour], count_uslov[dayofweek][hour], curIndex, to, num_virag_hash, hour, dayofweek);

	try
	{
		if(ind >= 0)
		{
			resvirag[num_virag_hash][ind]      = resusl;
			resvirag_hash[num_virag_hash][ind] = virag_hash[num_virag_hash][dayofweek][hour];
		}
	}
	catch(...)
	{
		int aa = 11;
	}

    return resusl;
}
//---------------------------------------------------------------------------
bool __fastcall TSmartTest::CheckExUsloviya(struct viragenie* uslov, int count_uslov, int* curIndex, struct techdata_order* to,
								   int num_virag_hash, int hour, int dayofweek)
{
   bool zn = false;
   if(count_uslov)
   {
	   zn = true;
	   for(int u = 0; u < count_uslov; u++)
	   {
		   float zn1;
		   int ind = curIndex[resvirag_tf];
		   if(OptimizAnalizPoUslov->Checked)
		   {
			   if(ind >= 0 && resvirag_sub_hash[num_virag_hash][u][ind] == uslov[u].viragenie_sub_hash /*virag_sub_hash[num_virag_hash][u][dayofweek][hour]*/)
				   zn1 = resvirag_sub[num_virag_hash][u][ind];
			   else
			   {
					zn1 = CalcViragenie2(&(uslov[u]), 0, 0, curIndex, to);

					if(ind >= 0)
					{
						resvirag_sub[num_virag_hash][u][ind]      = zn1;
						resvirag_sub_hash[num_virag_hash][u][ind] = uslov[u].viragenie_sub_hash /*virag_sub_hash[num_virag_hash][u][dayofweek][hour]*/;
					}
			   }
		   }
		   else
			  zn1 = CalcViragenie2(&(uslov[u]), 0, 0, curIndex, to);

		   if(!zn1)
			  return false;

		   zn = zn && zn1;
	   }
   }
   return zn;  //если нет условий должен возвращать false
}
//---------------------------------------------------------------------------
float __fastcall TSmartTest::CalcViragenie2(struct viragenie* vir, int subbackindexs,
											int variate, int* curIndex, struct techdata_order* to)
{
   if(!vir) return 0;

   struct viragenie* vir2 = vir;
   float zn = 0;

   //цикл плавающих условий, если одно из них выполнится, то возвращаем 1
   for(int backind = 0 - subbackindexs; backind <= 0; backind++)
   {
	  vir = vir2;
	  zn = 0;

	  if(vir->val == NULL && vir->zn)
		 int aa = 11;

	  while(1)
	  {
		 float val = 0;
		 if(vir->sub)
		 {
			if(vir->sub && vir->val == NULL && vir->zn)
			   int aa = 11;
			val = CalcViragenie2(vir->sub, vir->subbackindexs, (subbackindexs ? backind : variate), curIndex, to);
			vir->countsubexec++;
			if(val) vir->countsubtrue++;

			if(vir->sub && vir->val == NULL && vir->zn)
			   int aa = 11;
		 }
		 else
		 {
			if(vir->IsNumericVal())
			  val = vir->GetZn(to->CurActualPrice);
			else
			{
				//удален код
				
				if(vir->id_val_indicator < ID_INDICATOR)
				{
					if(vir->id_val_indicator == ID_CONST_VAL__TP)
					   val = fGetPerConstOrTable("TP");
					if(vir->id_val_indicator == ID_CONST_VAL__SL)
					   val = fGetPerConstOrTable("SL");

					if(vir->id_val_indicator > ID_DYNAMIC_VAL)
					{
						switch(vir->id_val_indicator)
						{
							case ID_DYNAMIC_VAL__OTRADE:          val = to->OTrade; break;
							case ID_DYNAMIC_VAL__SETCOLOR:        if(vir->col != clBlack && vir->col != clGray)
																	 CurColorOrder = vir->col;
                                                                  val = 1;
																  break;
							case ID_DYNAMIC_VAL__SET_TP:          if(vir->gnp(1))
																	 CurTPOrder = vir->gnp(1);
                                                                  val = 1;
																  break;
							case ID_DYNAMIC_VAL__TIME_AFTERMAXRES: val = to->timeaftermaxres; break;
							case ID_DYNAMIC_VAL__PRICELASTBUY1:  val = to->PrcieLastBuy1; break;
							case ID_DYNAMIC_VAL__PRICELASTBUY2:   val = to->PrcieLastBuy2; break;
							case ID_DYNAMIC_VAL__PRICELASTSELL1:   val = to->PrcieLastSell1; break;
							case ID_DYNAMIC_VAL__PRICELASTSELL2:    val = to->PrcieLastSell2; break;
							case ID_DYNAMIC_VAL__TIME_AFTERLASTBUY1: val = to->TimeAfterLastBuy1; break;
							case ID_DYNAMIC_VAL__TIME_AFTERLASTBUY2:  val = to->TimeAfterLastBuy2; break;
							case ID_DYNAMIC_VAL__TIME_AFTERLASTSELL1:  val = to->TimeAfterLastSell1; break;
							case ID_DYNAMIC_VAL__TIME_AFTERLASTSELL2:  val = to->TimeAfterLastSell2; break;
						}
                    }
				}
				else
				{
                    AnsiString vval = (AnsiString)vir->val;
					val = fGetDataInstr(vval, vir, (subbackindexs ? backind : variate), curIndex, to);  //здесь все индикаторы
                }
			}
		 }

		 if(vir->before_operation == '+' || vir->before_operation == '\0')
			zn += val;
		 else {
		 if(vir->before_operation == '-')
			zn -= val;
		 else {

		 if(vir->before_operation == '<')
			zn = zn < val;
		 else {
		 if(vir->before_operation == '>')
			zn = zn > val;
		 else {

		 if(vir->before_operation == '*')
			zn *= val;
		 else {
		 if(vir->before_operation == '/')
		 {
			if(val != 0)
			   zn /= val;
			else zn = 0;
		 }
		 else {

		 if(vir->before_operation == '&')
			zn = zn && val;
		 else {
		 if(vir->before_operation == '|')
			zn = zn || val;
		 else {
		 if(vir->before_operation == '=')
			zn = zn == val;
		 else {
		 if(vir->before_operation == '!')
			zn = (zn != val);
		 else {
		 if(vir->before_operation == 1)
			zn = zn <= val;
		 else {
		 if(vir->before_operation == 2)
			zn = zn >= val;
		 else {
		 if(vir->before_operation == 3)
			zn = zn != val;
		 else

		 if(vir->before_operation == '^')
			zn = pow(zn, val);
		 }
		 }
		 }
		 }
		 }
		 }
		 }
		 }
		 }
		 }
		 }
		 }

		 // Если это ноль и следующая операция и, то вернём false
		 // Это ускорит анализ
		 if(zn == 0 && vir->next && vir->next->before_operation == '&')
			return zn;

		 // Если это не ноль и следующая операция или, то вернём true
         // Это ускорит анализ и можно использовать цвета условий
		 if(zn != 0 && vir->next && vir->next->before_operation == '|')
            return zn;

		 if(vir->next) vir = vir->next;
		 else break;
	  }

	  if(zn) return zn;
   }

   return zn;
}
//---------------------------------------------------------------------------

//удален код

//---------------------------------------------------------------------------
// Сумма дельт
float* __fastcall TSmartTest::DELTA(int count, struct candlesdata* cndls,
								  struct timedaymonthyear_candles* tmdmy,
								  int Period)
{
    if(!cndls || count <= 0) return NULL;
	float* Out = (float*)malloc(sizeof(float)*count);
	ZeroMemory(Out, sizeof(float)*count);

	for(int i = 0; i < count; i++)
	{
		if( i >= Period - 1 )
		{
			float sum = 0;
			int ch = 0;
			for(int n = i - Period + 1; n <= i; n++)
			{
				sum = sum + Value(n, ID_DYNAMIC_VAL__CLOSE, cndls) - Value(n, ID_DYNAMIC_VAL__OPEN, cndls);
			}

			Out[i] = sum; 
		}
		else Out[i] = 0;
	}

	return Out;
}
//---------------------------------------------------------------------------
// Сумма производных дельт
float* __fastcall TSmartTest::DIFFDELTA(int count, struct candlesdata* cndls,
								  struct timedaymonthyear_candles* tmdmy,
								  int Period)
{
    if(!cndls || count <= 0) return NULL;
	float* Out = (float*)malloc(sizeof(float)*count);
	ZeroMemory(Out, sizeof(float)*count);

	for(int i = 0; i < count; i++)
	{
		if( i >= Period - 2 )
		{
			float sum = 0;
			int ch = 0;
			for(int n = i - Period + 1; n <= i; n++)
			{
				sum = sum + (Value(n, ID_DYNAMIC_VAL__CLOSE, cndls) - Value(n, ID_DYNAMIC_VAL__OPEN, cndls)) -
				            (Value(n - 1, ID_DYNAMIC_VAL__CLOSE, cndls) - Value(n - 1, ID_DYNAMIC_VAL__OPEN, cndls));
			}

			Out[i] = sum;
		}
		else Out[i] = 0;
	}

	return Out;
}
//---------------------------------------------------------------------------
// Сумма сопротивлений движению
float* __fastcall TSmartTest::DRAG(int count, struct candlesdata* cndls,
								  struct timedaymonthyear_candles* tmdmy,
								  int Period)
{
    if(!cndls || count <= 0) return NULL;
	float* Out = (float*)malloc(sizeof(float)*count);
	ZeroMemory(Out, sizeof(float)*count);

	for(int i = 0; i < count; i++)
	{
		if( i >= Period - 1 )
		{
			float sum = 0;
			int ch = 0;
			for(int n = i - Period + 1; n <= i; n++)
			{
				sum = sum + Value(n, ID_DYNAMIC_VAL__CLOSE, cndls) - Value(n, ID_DYNAMIC_VAL__HIGH, cndls) +
				            Value(n, ID_DYNAMIC_VAL__CLOSE, cndls) - Value(n, ID_DYNAMIC_VAL__LOW, cndls);
			}

			Out[i] = sum; 
		}
		else Out[i] = 0;
	}

	return Out;
}
//---------------------------------------------------------------------------

float* __fastcall TSmartTest::SMA(int count, struct candlesdata* cndls,
								  struct timedaymonthyear_candles* tmdmy,
								  int Period, AnsiString VType, AnsiString dayofweek)
{
    if(!cndls || count <= 0) return NULL;
	float* Out = (float*)malloc(sizeof(float)*count);
	ZeroMemory(Out, sizeof(float)*count);

	// Определим тип цены
	int vtype = DetermineTypePrice(VType);
	int dow = 0;
	if(dayofweek == "Pn")  dow = 1;
	if(dayofweek == "Vt")  dow = 2;
	if(dayofweek == "Sr")  dow = 3;
	if(dayofweek == "Cht") dow = 4;
	if(dayofweek == "Pt")  dow = 5;
	if(dayofweek == "Sb")  dow = 6;
	if(dayofweek == "Vs")  dow = 7;

	for(int i = 0; i < count; i++)
	{
	   if( i >= Period - 1 )
	   {
		  float sum = 0;
		  int ch = 0;
		  for(int n = i - Period + 1; n <= i; n++)
		  {
			  if(dow <= 0 || dow == tmdmy[n].dayofweek)
			  {
				  sum = sum + Value(n, vtype /*VType*/, cndls);
				  ch++;
			  }
		  }

		  Out[i] = sum/(float)ch /*(float)Period*/;
	   }
	   else Out[i] = 0;
	}
	return Out;
}

float* __fastcall TSmartTest::EMA(int count, struct candlesdata* cndls,
								  int Period, AnsiString VType)
{
    if(!cndls || count <= 0) return NULL;
	float* Out = (float*)malloc(sizeof(float)*count);
	ZeroMemory(Out, sizeof(float)*count);

	// Определим тип цены
	int vtype = DetermineTypePrice(VType);

	for(int i = 0; i < count; i++)
	{
		if(i == 0) Out[i] = Value(i, vtype /*VType*/, cndls);
		else       Out[i] = (Out[i - 1]*(Period - 1) + 2*Value(i, vtype /*VType*/, cndls)) /
						      (Period + 1);
	}
	return Out;
}



float __fastcall TSmartTest::Value(int I, int vtype /*AnsiString VType*/, void* dt)
{
    struct candlesdata* dt1 = (struct candlesdata*)dt;
	float*              dt2 = (float*)dt;

    if(I < 0) I = 0;

	switch(vtype)
	{
		case ID_DYNAMIC_VAL__CLOSE:    return dt1[I].priceclose;
		case ID_DYNAMIC_VAL__OPEN:     return dt1[I].priceopen;
		case ID_DYNAMIC_VAL__HIGH:     return dt1[I].high;
		case ID_DYNAMIC_VAL__LOW:      return dt1[I].low;
		case ID_DYNAMIC_VAL__VOLUME:   return dt1[I].volume;
		case ID_DYNAMIC_VAL__DIFF:     return dt1[I].high - dt1[I].low;

		case ID_DYNAMIC_VAL__HACLOSE:  return (dt1[I].priceopen + dt1[I].priceclose +
											   dt1[I].high + dt1[I].low)/4.0;
		case ID_DYNAMIC_VAL__HAOPEN:   if(I < 1) return dt1[I].priceopen;
		                               else      return (dt1[I - 1].priceopen + dt1[I - 1].priceclose)/2.0;
		case ID_DYNAMIC_VAL__HAHIGH:   return (dt1[I].priceopen + dt1[I].priceclose + dt1[I].high)/3.0;
		case ID_DYNAMIC_VAL__HALOW:    return (dt1[I].priceopen + dt1[I].priceclose + dt1[I].low)/3.0;
		case ID_DYNAMIC_VAL__HAMEDIAN: return (dt1[I].priceopen + dt1[I].priceclose + (dt1[I].high + dt1[I].low)/2.0)/3.0;

		case ID_DYNAMIC_VAL__MEDIAN:   return (dt1[I].high + dt1[I].low)/2.0;
		case ID_DYNAMIC_VAL__TYPICAL:  return (dt1[I].high + dt1[I].low + dt1[I].priceclose)/3.0;
		case ID_DYNAMIC_VAL__WEIGHTED: return (dt1[I].high + dt1[I].low + dt1[I].priceclose + dt1[I].priceopen)/4.0;
		case ID_DYNAMIC_VAL__KDIFF:    return (dt1[I].priceopen + 2.0*dt1[I].priceclose)/3.0;;
		case ID_DYNAMIC_VAL__ANY:      return dt2[I];
    }

	return 0;
}

// Определим тип цены
int __fastcall TSmartTest::DetermineTypePrice(AnsiString VType)
{
	if(VType == "haopen")   return ID_DYNAMIC_VAL__HAOPEN;
	if(VType == "haclose")  return ID_DYNAMIC_VAL__HACLOSE;
	if(VType == "hahigh")   return ID_DYNAMIC_VAL__HAHIGH;
	if(VType == "halow")    return ID_DYNAMIC_VAL__HALOW;
	if(VType == "hamedian") return ID_DYNAMIC_VAL__HAMEDIAN;

	if(VType == "O" || VType == "Open")   return ID_DYNAMIC_VAL__OPEN;
	if(VType == "C" || VType == "Close")  return ID_DYNAMIC_VAL__CLOSE;
	if(VType == "H" || VType == "High")   return ID_DYNAMIC_VAL__HIGH;
	if(VType == "L" || VType == "Low")    return ID_DYNAMIC_VAL__LOW;
	if(VType == "V" || VType == "Volume") return ID_DYNAMIC_VAL__VOLUME;
	if(VType == "HL") return ID_DYNAMIC_VAL__HAOPEN;

	if(VType == "M" || VType == "Median") return ID_DYNAMIC_VAL__MEDIAN;
	if(VType == "T" || VType == "Typical") return ID_DYNAMIC_VAL__TYPICAL;
	if(VType == "W" || VType == "Weighted") return ID_DYNAMIC_VAL__WEIGHTED;
	if(VType == "D" || VType == "Difference") return ID_DYNAMIC_VAL__DIFF;
	if(VType == "K")                          return ID_DYNAMIC_VAL__KDIFF;
	if(VType == "A" || VType == "Any")        return ID_DYNAMIC_VAL__ANY;

	return 0;
}

float __fastcall TSmartTest::rounding(float num, AnsiString tround)
{
   if(tround != "ON") return num;
   int round = 0;
   float mult = 10^round;
	if(num >= 0) return floor(num * mult + 0.5) / mult;
	else return ceil(num * mult - 0.5) / mult;
   return num;
}


float* __fastcall TSmartTest::MACDH(int count, struct candlesdata* cndls,
									int perfast, int perslow,
									AnsiString TypePrice, AnsiString Metod1,
									AnsiString Metod2, int persignal)
{
   float* macdh1   = (float*)malloc(sizeof(float)*count);
   float* EMA_TMP1 = NULL; 
   float* EMA_TMP2 = NULL; 
   float* t_MACD   = (float*)malloc(sizeof(float)*count);
   ZeroMemory(macdh1,   sizeof(float)*count);
   ZeroMemory(t_MACD,   sizeof(float)*count);  


   if(Metod1 == "EMA")
   {
	  EMA_TMP1 = (float*)malloc(sizeof(float)*count);
	  EMA_TMP2 = (float*)malloc(sizeof(float)*count);
	  ZeroMemory(EMA_TMP1, sizeof(float)*count);
	  ZeroMemory(EMA_TMP2, sizeof(float)*count);
   }
   if(Metod1 == "EFMA")
   {
	  EMA_TMP1 = (float*)malloc(sizeof(struct efma_data));
	  EMA_TMP2 = (float*)malloc(sizeof(struct efma_data));
	  ZeroMemory(EMA_TMP1, sizeof(struct efma_data));
	  ZeroMemory(EMA_TMP2, sizeof(struct efma_data));
   }

   float* EMA_TMP3 = NULL;
   if(Metod2 == "EMA")
   {
	  EMA_TMP3 = (float*)malloc(sizeof(float)*count);
	  ZeroMemory(EMA_TMP3, sizeof(float)*count);
   }

   for(int i = 0; i < count; i++)
   {
	  struct float3 fl3 = MACDH(i, perfast, perslow, TypePrice, cndls, "OFF",
								Metod1, Metod2, persignal,
								EMA_TMP1, EMA_TMP2, EMA_TMP3, t_MACD);
	  macdh1[i] = fl3.a;
   }              

   free(EMA_TMP1);
   free(EMA_TMP2);
   if(EMA_TMP3) free(EMA_TMP3);
   free(t_MACD);

   return macdh1;
}
//---------------------------------------------------------------------------

struct float3 __fastcall TSmartTest::MACDH(int I, int SP, int LP, AnsiString VT,
							 struct candlesdata* dt, AnsiString round,
							 AnsiString Metod, AnsiString SiM, int SiP,
							 float* EMA_TMP1, float* EMA_TMP2, float* EMA_TMP3, float* t_MACD) //--MACD Histogram ("MACDH")
{
   float Out = 0;

   if (SiM != "SMA" && SiM != "EMA") SiM="SMA";
   struct float3 fl3 = MACD(I, SP, LP, VT, dt, round, Metod, SiM, SiP,
							EMA_TMP1, EMA_TMP2, EMA_TMP3, t_MACD);
   Out = fl3.a;
   float Signal = fl3.b;

   if (I >= Max(SP, LP) + SiP - 2)
	  return tofloat3(rounding(Out - Signal, round), rounding(Signal, round), 0);
   else
	  return tofloat3(0, 0, 0);
}


struct float3 __fastcall TSmartTest::tofloat3(float a, float b, float c)
{
   struct float3 r;
   r.a = a; r.b = b; r.c = c;
   return r;
}

struct float3 __fastcall TSmartTest::MACD(int I, int ShortP, int LongP, AnsiString VType,
							 struct candlesdata* dt, AnsiString round,
							 AnsiString Metod, AnsiString SM, int SP,
							 float* EMA_TMP1, float* EMA_TMP2, float* EMA_TMP3,
							 float* t_MACD) //--Moving Average Convergence/Divergence ("MACD")
{
   float Out = 0;
   int Percent = 1;

   if (SM != "SMA" && SM != "EMA") SM = "SMA";
   float So = MA(I, ShortP, VType, dt, round, Metod, EMA_TMP1);
   float Lo = MA(I, LongP,  VType, dt, round, Metod, EMA_TMP2);
   int   i = I - Max(ShortP, LongP) + 1;
   if(i >= 0)
   {                       t_MACD[i-1];
	  if( Percent == 0 )
		t_MACD[i] = So - Lo;
	  else
	  {
		 if(Lo == 0) t_MACD[i] = 0;
		 else
	        t_MACD[i] = 100*(So - Lo) / Lo;
	  }

	  Out = MA(i, SP, "Any", t_MACD, round, SM, EMA_TMP3);
   }
   if(i < 0) i = 0;
   return tofloat3(rounding(t_MACD[i], round),rounding(Out, round),0);
}


float __fastcall TSmartTest::MA(int I, int Period, AnsiString VType,
								void* dt, AnsiString round,
								AnsiString Metod, float* EMA_TMP) //--Moving Average ("MA")
{
    float Out = 0;
	if( Metod == "SMA" )
		Out = tSMA(I, Period, VType, dt, round);
	if( Metod == "EMA" )
		Out = tEMA(I, Period, VType, dt, round, EMA_TMP);
	if( Metod == "EFMA" )
		Out = tEFMA(I, Period, VType, dt, round, (struct efma_data*)EMA_TMP);

	return rounding(Out, round);
}

//------------------------------------------------------------------
//--Moving Average SMA, EMA, VMA, SMMA, VMA
//------------------------------------------------------------------
float __fastcall TSmartTest::tSMA(int I, int Period, AnsiString VType,
								 void* dt, AnsiString round)
{
	float Out = 0;

	// Определим тип цены
	int vtype = DetermineTypePrice(VType);

	if( I >= Period - 1 )
	{
		float sum = 0;
		for (int i = I - Period + 1; i <= I; i++)
			sum = sum + Value(i, vtype /*VType*/, dt);

		Out = sum/Period;
	}
	return rounding(Out, round);
}


float __fastcall TSmartTest::tEMA(int I, int Period, AnsiString VType,
								 void* dt, AnsiString round, float* EMA_TMP)
{
    float Out = 0;

	// Определим тип цены
	int vtype = DetermineTypePrice(VType);

	if( I == 0 )
		EMA_TMP[I] = rounding(Value(I, vtype /*VType*/, dt),round);
	else
		EMA_TMP[I] = rounding((EMA_TMP[I - 1]*(Period - 1) + 2*Value(I, vtype /*VType*/, dt)) / (Period+1),round);

	if( I >= Period - 1 )
		Out = EMA_TMP[I];
	return rounding(Out,round);
}

void __fastcall TSmartTest::FormDestroy(TObject *Sender)
{
   SmartTest = NULL;
}
//---------------------------------------------------------------------------


float* __fastcall TSmartTest::RSI(int count, struct candlesdata* cndls,
								  int Period, AnsiString VType, AnsiString round) //--Relative Strength I("RSI")
{
	float* Up   = (float*)malloc(sizeof(float)*count);
	float* Down = (float*)malloc(sizeof(float)*count);
	float* val_Up   = (float*)malloc(sizeof(float)*count);
	float* val_Down = (float*)malloc(sizeof(float)*count);
	float* rsi      = (float*)malloc(sizeof(float)*count);
	ZeroMemory(Up,       sizeof(float)*count);
	ZeroMemory(Down,     sizeof(float)*count);
	ZeroMemory(val_Up,   sizeof(float)*count);
	ZeroMemory(val_Down, sizeof(float)*count);
	ZeroMemory(rsi,      sizeof(float)*count);

	// Определим тип цены
	int vtype = DetermineTypePrice(VType);

	float Out = 0;
	int P     = Period;
	AnsiString VT = VType;
	AnsiString R  = round;

	for(int I = 0; I < count; I++)
	{
		Out = 0;
		if( I == 0 )
		{
			Up[I]   = 0;
			Down[I] = 0;
		}
		if( I > 0 )
		{
			float Val     = Value(I, vtype /*VT*/, cndls);
			float ValPrev = Value(I - 1, vtype /*VT*/, cndls);
			if( ValPrev < Val )
				Up[I] = Val - ValPrev;
			else
				Up[I] = 0;

			if( ValPrev > Val )
				Down[I] = ValPrev - Val;
			else
				Down[I] = 0;

			if( (I == P - 1) || (I == P/* + 1*/) )
			{
				float sumU = 0;
				float sumD = 0;
				for(int i = I - P + 1; i <= I; i++ )
				{
					sumU = sumU + Up[i];
					sumD = sumD + Down[i];
				}
				val_Up[I]   = sumU/P;
				val_Down[I] = sumD/P;
			}
			if( I > P/*+1*/ )
			{
				val_Up[I]   = (val_Up[I-1] * (P-1) + Up[I]) / P;
				val_Down[I] = (val_Down[I-1] * (P-1) + Down[I]) / P;
			}
			if( I >= P - 1 )
			{
				Out = 100 / (1 + (val_Down[I] / val_Up[I]));
				//return rounding(Out, R)
				Out = rounding(Out, R);
			}
		}
		rsi[I] = Out;
	}
	free(Up);
	free(Down);
	free(val_Up);
	free(val_Down);
	return rsi;
}


struct float_f3* __fastcall TSmartTest::BB(int count, struct candlesdata* cndls,
										  int Period, AnsiString Metod,
										  float Shift, AnsiString VType,
								          AnsiString round) //--Bollinger Bands ("BB")
{
   float* BB_1 = (float*)malloc(sizeof(float)*count);
   float* BB_2 = (float*)malloc(sizeof(float)*count);
   float* BB_3 = (float*)malloc(sizeof(float)*count);
   ZeroMemory(BB_1, sizeof(float)*count);
   ZeroMemory(BB_2, sizeof(float)*count);
   ZeroMemory(BB_3, sizeof(float)*count);

   float* EMA_TMP = (float*)malloc(sizeof(float)*count);
   ZeroMemory(EMA_TMP, sizeof(float)*count);

   struct float_f3* res = (struct float_f3*)malloc(sizeof(struct float_f3));
   res->a = BB_1;
   res->b = BB_2;
   res->c = BB_3;

   int P         = Period;
   AnsiString M  = Metod;
   int S         = Shift;
   AnsiString VT = VType;
   AnsiString R  = round;

   for(int I = 0; I < count; I++)
   {
	  float b_ma = MA(I, P, VT, cndls, R, M, EMA_TMP);
	  float b_sd = tSD(I, P, VT, cndls, R, "SMA");
	  if( I >= P - 1 && b_ma && b_sd )
	  {
		  BB_1[I] = rounding(b_ma, R);
		  BB_2[I] = rounding(b_ma + S*b_sd, R);
		  BB_3[I] = rounding(b_ma - S*b_sd, R);
	  }
	  else
	  {
		  BB_1[I] = 0;
		  BB_2[I] = 0;
		  BB_3[I] = 0;
      }
   }
   free(EMA_TMP);
   return res;
}

float __fastcall TSmartTest::tSD(int I, int Period, AnsiString VType,
								void* dt, AnsiString round,
								AnsiString Metod) //--Standard Deviation ("SD")
{
   float Out = 0;
   int P         = Period;
   AnsiString M  = Metod;
   AnsiString VT = VType;
   AnsiString R  = round;
   float sum = 0;
   float t_ma = MA(I, P, VT, dt, R, M, NULL);

	// Определим тип цены
	int vtype = DetermineTypePrice(VType);

   if( I >= P - 1 && t_ma )
   {
	  for(int i = I - P + 1; i <= I; i++)
		sum = sum + pow((Value(i, vtype /*VT*/, dt) - t_ma), 2);

	  Out = sqrt(sum / P);
	  return rounding(Out, R);
   }
   else
	  return 0;
}


float* __fastcall TSmartTest::ATR(int count, struct candlesdata* cndls,
								  int Period, AnsiString round) //--Average True Range ("ATR")
{

   float* ATR = (float*)malloc(sizeof(float)*count);
   ZeroMemory(ATR, sizeof(float)*count);

	float Out = 0;
	int P        = Period; 
	AnsiString R = round; 

    for(int I = 0; I < count; I++)
	{

	   if( I < P - 1 )
		  ATR[I] = 0;
	   else
	   {
		  if( I == P - 1 )
		  {
			 float sum = 0;
			 for(int i = 0; i < P; i++ )
				sum = sum + TR(i, "off", cndls);

			 ATR[I] = sum / P;
		  }
		  else
		  {
			if( I > P - 1 )
		    ATR[I] = (ATR[I - 1] * (P - 1) + TR(I, "off", cndls)) / P;
		  }
	   }
	   if( I >= P - 1 )
	   {
		  Out = ATR[I];
	   }
	}
	return ATR;
}

float __fastcall TSmartTest::TR(int I, AnsiString round, struct candlesdata* cndls) //--True Range ("TR")
{
   AnsiString R = round; 
   float Out = 0;
   if( I == 0 )
	   Out = fabs(Value(I, ID_DYNAMIC_VAL__DIFF /*"Difference"*/, cndls));
   else
   {
	   Out = Max(fabs(Value(I, ID_DYNAMIC_VAL__DIFF /*"Difference"*/, cndls)), Max(
				 fabs(Value(I, ID_DYNAMIC_VAL__HIGH /*"High"*/, cndls) - Value(I - 1, ID_DYNAMIC_VAL__CLOSE /*"Close"*/, cndls)),
				 fabs(Value(I - 1, ID_DYNAMIC_VAL__CLOSE /*"Close"*/, cndls) - Value(I, ID_DYNAMIC_VAL__LOW /*"Low"*/, cndls)) ) );
   }
   return rounding(Out, R);
}



float* __fastcall TSmartTest::Extremums(int count, struct candlesdata* cndls,
										int Period, AnsiString round)
{
	float* extr = (float*)malloc(sizeof(float)*count);
	ZeroMemory(extr, sizeof(float)*count);

	//float* cur

	for(int i = 1; i < count - 1; i++)
	{
	   if(Value(i, ID_DYNAMIC_VAL__LOW /*"Low"*/, cndls) <= Value(i - 1, ID_DYNAMIC_VAL__LOW /*"Low"*/, cndls) &&
		  Value(i, ID_DYNAMIC_VAL__LOW /*"Low"*/, cndls) <= Value(i + 1, ID_DYNAMIC_VAL__LOW /*"Low"*/, cndls) )
		  extr[i] = Value(i, ID_DYNAMIC_VAL__LOW /*"Low"*/, cndls);
	   if(Value(i, ID_DYNAMIC_VAL__HIGH /*"High"*/, cndls) >= Value(i - 1, ID_DYNAMIC_VAL__HIGH /*"High"*/, cndls) &&
		  Value(i, ID_DYNAMIC_VAL__HIGH /*"High"*/, cndls) >= Value(i + 1, ID_DYNAMIC_VAL__HIGH /*"High"*/, cndls) )
		  extr[i] = Value(i, ID_DYNAMIC_VAL__HIGH /*"High"*/, cndls);
	}
	return extr;
}



struct float_f3* __fastcall TSmartTest::PC(int count, struct candlesdata* cndls,
								           int Period, AnsiString round)  //--Price Channel ("PC")
{
	float* a = (float*)malloc(sizeof(float)*count);
	float* b = (float*)malloc(sizeof(float)*count);
	float* c = (float*)malloc(sizeof(float)*count);
	ZeroMemory(a, sizeof(float)*count);
	ZeroMemory(b, sizeof(float)*count);
	ZeroMemory(c, sizeof(float)*count);
	struct float_f3* res = (struct float_f3*)malloc(sizeof(struct float_f3));
	ZeroMemory(res, sizeof(struct float_f3));
	res->a = a;
	res->b = b;
	res->c = c;

	int P = Period;

	for(int I = 0; I < count; I++)
	{
	   float PCh = 0;
	   float PCl = 0;
	   float PCm = 0;

	   if( I >= P - 1 )
	   {
		   for(int n = I - P + 1; n <= I; n++)
		   {
			  if(Value(n, ID_DYNAMIC_VAL__HIGH /*"High"*/, cndls) > PCh)
				 PCh = Value(n, ID_DYNAMIC_VAL__HIGH /*"High"*/, cndls);
			  if(Value(n, ID_DYNAMIC_VAL__LOW /*"Low"*/, cndls) < PCl || PCl == 0)
				 PCl = Value(n, ID_DYNAMIC_VAL__LOW /*"Low"*/, cndls);
           }

		   PCm = (PCh + PCl)/2;
	   }
	   a[I] = rounding(PCh, round);
	   b[I] = rounding(PCl, round);
	   c[I] = rounding(PCm, round);
	}

	return res;
}

//средняя разница между средними ценами соседних свечек
float* __fastcall TSmartTest::EDA(int count, struct candlesdata* cndls,
								  int Period, AnsiString VType, AnsiString ABS, AnsiString round)
{
	float* eda = (float*)malloc(sizeof(float)*count);
	ZeroMemory(eda, sizeof(float)*count);
	bool babs = (ABS == "abs");

	// Определим тип цены
	int vtype = DetermineTypePrice(VType);

	if(!babs)
	for(int i = Period - 1 + 1; i < count; i++)
	{
	   float sred = 0;
	   for(int n = i - Period + 1; n <= i; n++)
		  sred += ( Value(i, vtype /*VType*/, cndls) - Value(i - 1, vtype /*VType*/, cndls) );
	   sred /= Period;
	   eda[i] = sred;
	}
	if(babs)
	for(int i = Period - 1 + 1; i < count; i++)
	{
	   float sred = 0;
	   for(int n = i - Period + 1; n <= i; n++)
		  sred += fabs( Value(i, vtype /*VType*/, cndls) - Value(i - 1, vtype /*VType*/, cndls) );
	   sred /= Period;
	   eda[i] = sred;
	}
	return eda;
}


//средняя разница между средними ценами соседних свечек, чтобы не ставить, когда консолидация
float* __fastcall TSmartTest::EDAR(int count, struct candlesdata* cndls,
								  int Period, AnsiString VType, AnsiString ABS, AnsiString round)
{
	float* eda = (float*)malloc(sizeof(float)*count);
	ZeroMemory(eda, sizeof(float)*count);
	bool babs = (ABS == "abs");

	// Определим тип цены
	int vtype = DetermineTypePrice(VType);

	if(!babs)
	for(int i = Period - 1 + 1; i < count; i++)
	{
	   float sred = 0;
	   for(int n = i - Period + 1; n <= i; n++)
		  sred += ( Value(n, vtype /*VType*/, cndls) - Value(n - 1, vtype /*VType*/, cndls) );
	   sred /= Period;
	   eda[i] = sred;
	}
	if(babs)
	for(int i = Period - 1 + 1; i < count; i++)
	{
	   float sred = 0;
	   for(int n = i - Period + 1; n <= i; n++)
		  sred += fabs( Value(n, vtype /*VType*/, cndls) - Value(n - 1, vtype /*VType*/, cndls) );
	   sred /= Period;
	   eda[i] = sred;
	}
	return eda;
}

//минимум за последние n свечек
float* __fastcall TSmartTest::LocMin(int count, struct candlesdata* cndls,
									 int Period, AnsiString VType, AnsiString round)
{
	float* locmin = (float*)malloc(sizeof(float)*count);
	ZeroMemory(locmin, sizeof(float)*count);

	// Определим тип цены
	int vtype = DetermineTypePrice(VType);

	for(int i = Period; i < count; i++)
	{
	   float min = 1000000;
	   for(int n = i - Period + 1; n <= i; n++)
		  if( Value(n, vtype /*VType*/, cndls) < min )
			 min = Value(n, vtype /*VType*/, cndls);
	   locmin[i] = min;
	}
	return locmin;
}
//максимум за последние n свечек
float* __fastcall TSmartTest::LocMax(int count, struct candlesdata* cndls,
									 int Period, AnsiString VType, AnsiString round)
{
	float* locmax = (float*)malloc(sizeof(float)*count);
	ZeroMemory(locmax, sizeof(float)*count);

	// Определим тип цены
	int vtype = DetermineTypePrice(VType);

	for(int i = Period; i < count; i++)
	{
	   float max = 0;
	   for(int n = i - Period + 1; n <= i; n++)
		  if( Value(n, vtype /*VType*/, cndls) > max )
			 max = Value(n, vtype /*VType*/, cndls);
	   locmax[i] = max;
	}
	return locmax;
}

//вспомогательная функция
float __fastcall TSmartTest::tb_sma(struct candlesdata* cndls, int k, int Period, AnsiString VType)
{
	float res = 0;
	int   ch  = 0;

	// Определим тип цены
	int vtype = DetermineTypePrice(VType);

	for(int i = k - Period + 1; i <= k; i++)
	{
	   res += Value(i, vtype /*VType*/, cndls);
	   ch++;
	}
	if(ch) res /= (float)ch;
	return res;
}

//время в секундах или кол-во свечек между точными экстремумами
//расчет до последней закрытой свечки минус период/2
//также может быть среднее время между экстремумами за n последнее количество промежутков между экстремумами
//или за последнее n_candles кол-во свечек
//diff - разница минимальная между экстремумами
float* __fastcall TSmartTest::TBEXTR(int count, struct candlesdata* cndls,
									 float Period, AnsiString VType, int n_candles,
									 float diff, AnsiString round/*,
									 int* pricepoints, int& count_pricepoints*/)
{
	float* tbextr = (float*)malloc(sizeof(float)*count);
	ZeroMemory(tbextr, sizeof(float)*count);

	count_pricepoints = 0;
	ZeroMemory(pricepoints, sizeof(struct extrems)*count_pricepoints);

	// Определим тип цены
	int vtype = DetermineTypePrice(VType);

	for(int i = 0/*Period*/; i < count /*- Period/2*/; i++)
	{
	   float st = i - n_candles;
	   if(st < Period/2.0 + 2.0) continue;

	   float curmax = 0;
	   float curmin = 1000000;
	   bool nowmaxextr = false;
	   bool nowminextr = false;
	   int ch = 0;
	   int count_cndls = 0;
	   __int64 lastmaxextremtochno = 0;
	   __int64 lastminextremtochno = 0;

	   //находим экстремумы
	   float end = (float)i - Period/2.0;
	   for(int k = st; k < end; k++)
	   {
		  //AnsiString VType1 = VType;
		  int vtype1 = vtype;
		  //if(VType == "HL") VType1 = "High";
		  if(vtype == ID_DYNAMIC_VAL__HIGH_LOW) vtype1 = ID_DYNAMIC_VAL__HIGH;
		  float t1 = Value(k + Period/2.0 - 2, vtype1 /*VType1*/, cndls); //tb_sma(cndls, k + Period/2.0 - 2, Period, VType);
		  float t2 = Value(k + Period/2.0 - 1, vtype1 /*VType1*/, cndls); //tb_sma(cndls, k + Period/2.0 - 1, Period, VType);
		  float t3 = Value(k + Period/2.0 - 0, vtype1 /*VType1*/, cndls); //tb_sma(cndls, k + Period/2.0 - 0, Period, VType);
		  //if(VType == "HL") VType1 = "Low";
		  if(vtype == ID_DYNAMIC_VAL__HIGH_LOW) vtype1 = ID_DYNAMIC_VAL__LOW;
		  float t1_2 = Value(k + Period/2.0 - 2, vtype1 /*VType1*/, cndls);
		  float t2_2 = Value(k + Period/2.0 - 1, vtype1 /*VType1*/, cndls);
		  float t3_2 = Value(k + Period/2.0 - 0, vtype1 /*VType1*/, cndls);
		  if(Period > 1)
		  {
			 AnsiString VType1 = VType;
			 //if(VType == "HL") VType1 = "High";
			 if(vtype == ID_DYNAMIC_VAL__HIGH_LOW) vtype1 = ID_DYNAMIC_VAL__HIGH;
			 t1 = tb_sma(cndls, k + Period/2.0 - 2, Period, vtype1 /*VType1*/);
			 t2 = tb_sma(cndls, k + Period/2.0 - 1, Period, vtype1 /*VType1*/);
			 t3 = tb_sma(cndls, k + Period/2.0 - 0, Period, vtype1 /*VType1*/);
			 //if(VType == "HL") VType1 = "Low";
			 if(vtype == ID_DYNAMIC_VAL__HIGH_LOW) vtype1 = ID_DYNAMIC_VAL__LOW;
			 t1_2 = tb_sma(cndls, k + Period/2.0 - 2, Period, vtype1 /*VType1*/);
			 t2_2 = tb_sma(cndls, k + Period/2.0 - 1, Period, vtype1 /*VType1*/);
			 t3_2 = tb_sma(cndls, k + Period/2.0 - 0, Period, vtype1 /*VType1*/);
          }
		  if(t2 > t1 && t2 > t3 && (t2 > curmax || !nowmaxextr))
		  {
			  curmax = t2;
			  if((nowminextr && curmax - curmin >= diff) || !lastmaxextremtochno)
			  {
				 nowmaxextr = true;
				 nowminextr = false;
				 if(lastminextremtochno && lastmaxextremtochno)
				 {
					count_cndls += abs(lastmaxextremtochno - lastminextremtochno);
					ch++;
					if(count_pricepoints < 100000)
					{
					pricepoints[count_pricepoints].price = t2;
					pricepoints[count_pricepoints].index = k;
					count_pricepoints++;
					}
				 }
				 lastmaxextremtochno = k;
			  }
		  }
		  if(t2_2 < t1_2 && t2_2 < t3_2 && (t2_2 < curmin || !nowmaxextr))
		  {
			  curmin = t2_2;
			  if((nowmaxextr && curmax - curmin >= diff) || !lastminextremtochno)
			  {
				 nowminextr = true;
				 nowmaxextr = false;
				 if(lastminextremtochno && lastmaxextremtochno)
				 {
					count_cndls += abs(lastmaxextremtochno - lastminextremtochno);
					ch++;
					if(count_pricepoints < 100000)
					{
					pricepoints[count_pricepoints].price = t2_2;
					pricepoints[count_pricepoints].index = k;
					count_pricepoints++;
                    }
				 }
				 lastminextremtochno = k;
			  }
          }
	   }
	   tbextr[i] = 0;
	   if(ch) tbextr[i] = (float)count_cndls/(float)ch;
    }

	
	return tbextr;
}

float* __fastcall TSmartTest::EAnews(int timeframe, int count, struct candlesdata* cndls,
									 int Period, AnsiString TextSearch, AnsiString round)
{
	float* eanews = (float*)malloc(sizeof(float)*count);
	ZeroMemory(eanews, sizeof(float)*count);

	struct news* n = &FMain->allnews[timeframe];
	int* cn        = FMain->candles_news[timeframe];

	for(int i = 0; i < count; i++)
	{
	   int cni = cn[i];
	   float sum = 0;
	   while(n[cni].timedate == cndls[i].timeopen)
	   {
		  if(TextSearch != "" && n[cni].text &&
		     ((AnsiString)(n[cni].text)).AnsiPos(TextSearch) <= 0)
		  {
			  cni++;
			  continue;
          }
		  sum += n[cni].volatiln;
		  cni++;
	   }
       eanews[i] = sum;
	}
	return eanews;
}

// Фильтрованное среднее, высокочастотным фильтром
float* __fastcall TSmartTest::EFMA(int count, struct candlesdata* cndls,
								   int Period, float gamma, AnsiString VType,
								   AnsiString efma_type,
								   AnsiString round)
{
	float* efma = (float*)malloc(sizeof(float)*count);
	ZeroMemory(efma, sizeof(float)*count);
	if(Period < 2 || Period > 10) return efma;

	bool b_exp = (efma_type == "exp" || efma_type == "EXP");

	float L0 = 0;
	float L1 = 0;
	float L2 = 0;
	float L3 = 0;
	float L4 = 0;
	float L5 = 0;
	float L6 = 0;
	float L7 = 0;
	float L8 = 0;
	float L9 = 0;
	float L10 = 0;

	float L0A = 0;
	float L1A = 0;
	float L2A = 0;
	float L3A = 0;
    float L4A = 0;
	float L5A = 0;
	float L6A = 0;
	float L7A = 0;
	float L8A = 0;
	float L9A = 0;
	float L10A = 0;

	// Определим тип цены
	int vtype = DetermineTypePrice(VType);

	for(int i = 0; i < count; i++)
	{
       L0A = L0;
	   L1A = L1;
	   L2A = L2;
	   L3A = L3;
	   L4A = L4;
	   L5A = L5;
	   L6A = L6;
	   L7A = L7;
	   L8A = L8;
	   L9A = L9;
	   L10A = L10;

	   L0 = (1 - gamma) * Value(i, vtype /*VType*/, cndls) + gamma * L0A;
	   L1 = - gamma * L0 + L0A + gamma * L1A;
	   L2 = - gamma * L1 + L1A + gamma * L2A;
	   L3 = - gamma * L2 + L2A + gamma * L3A;
	   L4 = - gamma * L3 + L3A + gamma * L4A;

	   L5 = - gamma * L4 + L4A + gamma * L5A;
	   L6 = - gamma * L5 + L5A + gamma * L6A;
	   L7 = - gamma * L6 + L6A + gamma * L7A;
	   L8 = - gamma * L7 + L7A + gamma * L8A;
	   L9 = - gamma * L8 + L8A + gamma * L9A;
	   L10 = - gamma * L9 + L9A + gamma * L10A;

	   float Filt = 0;
	   if(!b_exp)
	   {
		   if(Period == 2) Filt = (L0 + L1) / 2;
		   if(Period == 3) Filt = (L0 + 2*L1 + L2) / 4;
		   if(Period == 4) Filt = (L0 + 2*L1 + 2*L2 + L3) / 6;
		   if(Period == 5) Filt = (L0 + 2*L1 + 2*L2 + 2*L3 + L4) / 8;

		   if(Period == 6) Filt = (L0 + 2*L1 + 2*L2 + 2*L3 + 2*L4 + L5) / 10;
		   if(Period == 7) Filt = (L0 + 2*L1 + 2*L2 + 2*L3 + 2*L4 + 2*L5 + L6) / 12;
		   if(Period == 8) Filt = (L0 + 2*L1 + 2*L2 + 2*L3 + 2*L4 + 2*L5 + 2*L6 + L7) / 14;
		   if(Period == 9) Filt = (L0 + 2*L1 + 2*L2 + 2*L3 + 2*L4 + 2*L5 + 2*L6 + 2*L7 + L8) / 16;
		   if(Period == 10) Filt = (L0 + 2*L1 + 2*L2 + 2*L3 + 2*L4 + 2*L5 + 2*L6 + 2*L7 + 2*L8 + L9) / 18;
	   }
	   else
	   {
		   if(Period == 2) Filt = (L0 + L1) / 2;
		   if(Period == 3) Filt = (L0 + 2*L1 + 3*L2) / 6;
		   if(Period == 4) Filt = (L0 + 2*L1 + 3*L2 + 4*L3) / 10;
		   if(Period == 5) Filt = (L0 + 2*L1 + 3*L2 + 4*L3 + 5*L4) / 15;

		   if(Period == 6) Filt = (L0 + 2*L1 + 3*L2 + 4*L3 + 5*L4 + 6*L5) / 21;
		   if(Period == 7) Filt = (L0 + 2*L1 + 3*L2 + 4*L3 + 5*L4 + 6*L5 + 7*L6) / 28;
		   if(Period == 8) Filt = (L0 + 2*L1 + 3*L2 + 4*L3 + 5*L4 + 6*L5 + 7*L6 + 8*L7) / 36;
		   if(Period == 9) Filt = (L0 + 2*L1 + 3*L2 + 4*L3 + 5*L4 + 6*L5 + 7*L6 + 8*L7 + 9*L8) / 45;
		   if(Period == 10) Filt = (L0 + 2*L1 + 3*L2 + 4*L3 + 5*L4 + 6*L5 + 7*L6 + 8*L7 + 9*L8 + 10*L9) / 55;
       }
	   if(Filt < 50000 || Filt > 200000)
          int aa = 11;

	   float zn = Value(i, vtype /*VType*/, cndls);

	   if(Filt/zn < 0.98)
	      Filt = 0;
	   efma[i] = Filt;

	   if(Filt < 50000 || Filt > 150000)
          int aa = 11;
	}
	return efma;
}

//для macdh
float __fastcall TSmartTest::tEFMA(int I, int Period, AnsiString VType,
								   void* dt, AnsiString round,
								   struct efma_data* EFMA_TMP)
{
	   float gamma = 0.5;
	   struct efma_data* EF = EFMA_TMP;

	   EF->L0A = EF->L0;
	   EF->L1A = EF->L1;
	   EF->L2A = EF->L2;
	   EF->L3A = EF->L3;
	   EF->L4A = EF->L4;

	   // Определим тип цены
	   int vtype = DetermineTypePrice(VType);

	   EF->L0 = (1 - gamma) * Value(I, vtype /*VType*/, dt) + gamma * EF->L0A;
	   EF->L1 = - gamma * EF->L0 + EF->L0A + gamma * EF->L1A;
	   EF->L2 = - gamma * EF->L1 + EF->L1A + gamma * EF->L2A;
	   EF->L3 = - gamma * EF->L2 + EF->L2A + gamma * EF->L3A;
	   EF->L4 = - gamma * EF->L3 + EF->L3A + gamma * EF->L4A;

	   float Filt = 0;
	   if(Period == 2) Filt = (EF->L0 + EF->L1) / 2;
	   if(Period == 3) Filt = (EF->L0 + 2*EF->L1 + EF->L2) / 4;
	   if(Period == 4) Filt = (EF->L0 + 2*EF->L1 + 2*EF->L2 + EF->L3) / 6;
	   if(Period == 5) Filt = (EF->L0 + 2*EF->L1 + 2*EF->L2 + 2*EF->L3 + EF->L4) / 8;

	   if(Filt/Value(I, vtype /*VType*/, dt) < 0.98) Filt = 0;
	   return Filt;
}


float* __fastcall TSmartTest::MinMaxLast(int count, struct candlesdata* cndls,
										 AnsiString TypeMinMax,
										 int mindiffminmax, AnsiString round)
{
	struct indexes_extremums* ie = (struct indexes_extremums*)malloc(sizeof(struct indexes_extremums*));
	ZeroMemory(ie, sizeof(struct indexes_extremums));

	int* indexes = (int*)malloc(sizeof(int)*count);
	struct extremum* minmax = (struct extremum*)malloc(sizeof(struct extremum)*40000);
	ZeroMemory(indexes, sizeof(int)*count);
	ZeroMemory(minmax, sizeof(float)*40000);
	int count_extremums = 0;                         

	ie->extremums = minmax;
	ie->indexes   = indexes;

   //минимумы и максимумы чередуются друг за другом
   //ямка и гора
   //последний экстремум обновляется по мере поступления свечек, если это не другой экстремум
   //минимальная разница между экстремумами или их свечками, их сложные комбинации

   bool lastmin = false;
   bool lastmax = true;

   float curmin = 0;
   float curmax = 0;

   bool first = true;
   int last_index = 0;

   for(int i = 2; i < count; i++)
   {
	  struct candlesdata cndl1  = cndls[i - 2];
	  struct candlesdata cndl2  = cndls[i - 1];
	  struct candlesdata cndl3  = cndls[i];

	  if(i == 57)
         int aa = 11;
		 
	  if( cndl2.priceclose < cndl1.priceclose && cndl2.priceclose <= cndl3.priceclose) //)
	  {
		 float curval = 0;
		 curval = cndl2.priceclose;

		 //если был максимум, то запоминаем новый минимум, если был минимум, то его обновляем
		 if(lastmax)
		 {
			 //минимум после максимума, если между ними разница не менее mindiffextr
			 //и если уст.=ановлен чекбокс, то минимум меньше максимума и наоборот
			 if(fabs(curval - curmax) >= mindiffminmax  )
			 {
				 minmax[count_extremums].index = i - 1;
				 minmax[count_extremums].val   = curval;
				 minmax[count_extremums].min   = true;

				 indexes[i - 1] = count_extremums;
				 last_index = i - 1;

				 count_extremums++;
				 lastmin = true;
				 lastmax = false;
				 curmin = curval;
			 }
		 }
		 else
		 {
			 // Обновление старого минимума на новый
			 if(curval <= minmax[count_extremums - 1].val && count_extremums > 0)
			 {
				 minmax[count_extremums - 1].index = i - 1;
				 minmax[count_extremums - 1].val   = curval;
				 curmin = curval;

				 indexes[last_index] = 0;
				 indexes[i - 1] = count_extremums - 1;
				 last_index = i - 1;
			 }
         }
	  }

	  if( cndl2.priceclose > cndl1.priceclose && cndl2.priceclose >= cndl3.priceclose)
	  {
		 float curval = 0;
		 curval = cndl2.priceclose;

		 //если был максимум, то запоминаем новый минимум, если был минимум, то его обновляем
		 if(lastmin)
		 {
			 //максимум после минимума, если между ними разница не менее mindiffextr
			 //и если уст.=ановлен чекбокс, то минимум меньше максимума и наоборот
			 if(fabs(curmin - curval) >= mindiffminmax )
			 {
				 minmax[count_extremums].index = i - 1;
				 minmax[count_extremums].val   = curval;
				 minmax[count_extremums].min   = false;

				 indexes[i - 1] = count_extremums;
				 last_index = i - 1;

				 count_extremums++;
				 lastmin = false;
				 lastmax = true;
				 curmax = curval;
			 }
		 }
		 else
		 {
			 // Обновление старого максимума на новый
			 if(curval >= minmax[count_extremums - 1].val && count_extremums > 0)
			 {
				 minmax[count_extremums - 1].index = i - 1;
				 minmax[count_extremums - 1].val   = curval;
				 curmax = curval;

				 indexes[last_index] = 0;
				 indexes[i - 1] = count_extremums - 1;
				 last_index = i - 1;
			 }
		 }
	  }
   }
   return (float*)ie;    
}


float* __fastcall TSmartTest::ImpulseLightness(int count, struct candlesdata* cndls,
											   AnsiString round)
{
	float* res = (float*)malloc(sizeof(float)*count);
	ZeroMemory(res, sizeof(float)*count);

	// легкость импульса (+)плюс поддержка (-)минус сопротивление
	// (close - open/*)/V*/ + /*(*/open - low - (high - close))*V
	for(int i = 0; i < count; i++)
	{
	   res[i] = (cndls[i].priceclose - cndls[i].priceopen/*)/cndls[i].volume*/ +
				/*(*/cndls[i].priceopen - cndls[i].low - (cndls[i].high - cndls[i].priceclose))*cndls[i].volume;
	}
	return res;
}


//---------------------------------------------------------------------------
void __fastcall TSmartTest::ReSizeUslClick(TMemo* m)
{
	if(m->Height < 102)
	{
		m->Height = 280;
		if(m == ConstVals)
		{
			m->Height = 350;
			ButtonUlutshaizer->Top = m->Top + m->Height + 10;
			ButtonMassAnalize->Top = ButtonUlutshaizer->Top + ButtonUlutshaizer->Height + 10;
			ButtonUlutshaizer->BringToFront();
		    ButtonMassAnalize->BringToFront();
		}
		m->BringToFront();
	}
	else
	{
		m->Height = 101;
		if(m == ConstVals)
		{
			m->Height = 71;
			ButtonUlutshaizer->Top = 415;
			ButtonMassAnalize->Top = 464;
		}
        m->SendToBack();
	}
}
//---------------------------------------------------------------------------


void __fastcall TSmartTest::ReSizeUslPervBuyClick(TObject *Sender)
{
	ReSizeUslClick(UslPervBuy);
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::ReSizeUslPervSellClick(TObject *Sender)
{
	ReSizeUslClick(UslPervSell);
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::ReSizeUslVstrSellClick(TObject *Sender)
{
	ReSizeUslClick(UslVstrSell);
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::ReSizeUslVstrBuyClick(TObject *Sender)
{
	ReSizeUslClick(UslVstrBuy);
}
//---------------------------------------------------------------------------



void __fastcall TSmartTest::ButUslAllDaysClick(TObject *Sender)
{
	if(!Sender)
       return;

	ButUslAllDays->Font->Style = TFontStyles();
	ButUslPn->Font->Style = TFontStyles();
	ButUslVt->Font->Style = TFontStyles();
	ButUslSr->Font->Style = TFontStyles();
	ButUslCht->Font->Style = TFontStyles();
	ButUslPt->Font->Style = TFontStyles();

	CurUslTypeDay = 0;

	if(Sender == ButUslAllDays)
		CurUslTypeDay = USL_ALL_DAYS;
	if(Sender == ButUslPn)
		CurUslTypeDay = USL_PN;
	if(Sender == ButUslVt)
		CurUslTypeDay = USL_VT;
	if(Sender == ButUslSr)
		CurUslTypeDay = USL_SR;
	if(Sender == ButUslCht)
		CurUslTypeDay = USL_CHT;
	if(Sender == ButUslPt)
		CurUslTypeDay = USL_PT;

	((TSpeedButton*)Sender)->Font->Style = TFontStyles() << fsBold << fsUnderline;
	SetCurUslDayHour();
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::FormCreate(TObject *Sender)
{
	CurUslTypeDay = USL_ALL_DAYS;
	CurUslNumHour = USL_ALL_HOURS;
	ButUslAllDays->Font->Style = TFontStyles() << fsBold << fsUnderline;
	ButUslAllHours->Font->Style = TFontStyles() << fsBold << fsUnderline;
	SetCurUslDayHour();
}
//---------------------------------------------------------------------------
void __fastcall TSmartTest::SetCurUslDayHour()
{
	UslPervBuy->Text  = UslPervBuyTextDays[CurUslTypeDay - 1][CurUslNumHour - 1];
	UslPervSell->Text = UslPervSellTextDays[CurUslTypeDay - 1][CurUslNumHour - 1];
	UslVstrSell->Text = UslVstrSellTextDays[CurUslTypeDay - 1][CurUslNumHour - 1];
	UslVstrBuy->Text  = UslVstrBuyTextDays[CurUslTypeDay - 1][CurUslNumHour - 1];
	Psevdonimy->Text  = PsevdonimyTextDays[CurUslTypeDay - 1][CurUslNumHour - 1];
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::UslPervBuyChange(TObject *Sender)
{
	UslPervBuyTextDays[CurUslTypeDay - 1][CurUslNumHour - 1] = UslPervBuy->Text;
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::UslPervSellChange(TObject *Sender)
{
	UslPervSellTextDays[CurUslTypeDay - 1][CurUslNumHour - 1] = UslPervSell->Text;
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::UslVstrSellChange(TObject *Sender)
{
	UslVstrSellTextDays[CurUslTypeDay - 1][CurUslNumHour - 1] = UslVstrSell->Text;
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::UslVstrBuyChange(TObject *Sender)
{
	UslVstrBuyTextDays[CurUslTypeDay - 1][CurUslNumHour - 1] = UslVstrBuy->Text;
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::PsevdonimyChange(TObject *Sender)
{
	PsevdonimyTextDays[CurUslTypeDay - 1][CurUslNumHour - 1] = Psevdonimy->Text;
}
//---------------------------------------------------------------------------


void __fastcall TSmartTest::FormMouseEnter(TObject *Sender)
{
	TimerBlendDown->Enabled = false;
	TimerBlendUp->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::FormMouseLeave(TObject *Sender)
{
	if(Sender != this)
       return;

	TimerBlendDown->Enabled = true;
	TimerBlendUp->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::TimerBlendDownTimer(TObject *Sender)
{
	if(AlphaBlendValue <= 40 || zapret_alpha)
	{
		TimerBlendDown->Enabled = false;
		return;
	}
	AlphaBlendValue -=12;
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::TimerBlendUpTimer(TObject *Sender)
{
	if(AlphaBlendValue >= 240)
	{
		TimerBlendUp->Enabled = false;
        AlphaBlendValue = 240;
		return;
	}
	int ab = AlphaBlendValue + 40;
	if(ab > 240)
	   ab = 240;
	AlphaBlendValue = ab;
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::VseDniClick(TObject *Sender)
{
    Ponedelnik->Checked = VseDni->Checked;
	Vtornik->Checked    = VseDni->Checked;
	Sreda->Checked      = VseDni->Checked;
	Chetverg->Checked   = VseDni->Checked;
	Pyatnica->Checked   = VseDni->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::Button2Click(TObject *Sender)
{
    ReplacesPanel->Left = Button2->Left + Button2->Width - ReplacesPanel->Width;
	ReplacesPanel->BringToFront();
	ReplacesPanel->Visible = true;
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::Button4Click(TObject *Sender)
{
    ReplacesPanel->Visible = false;
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::Button3Click(TObject *Sender)
{
	AnsiString rtext = MemoReplaces->Text;

	// Вычислим все переменные и змены им
	AnsiString vals[100];
	AnsiString valszn[100][2];
	int ct = 0, ctvalzn = 0;
	Explode(vals, ct, rtext, "\n");

    for(int i = 0; i < ct; i++)
	{
		AnsiString zn[2];
		int ctzn = 0;
		Explode(zn, ctzn, vals[i], "=");

		if(ctzn > 0 & zn[0] != "")
		{
			if(ctzn > 1 && zn[1].Length() > 0 && zn[1][zn[1].Length() - 1] == '\r')
			   zn[1].SetLength(zn[1].Length() - 1);

			valszn[ctvalzn][0] = zn[0];
			valszn[ctvalzn][1] = zn[1];
			ctvalzn++;
        }
	}

    // Заменим везде в условиях для всех дней и во всех псевдонимах
	for(int i = 0; i < 6; i++)
	{
		for(int h = 0; h < 15; h++)
		{
			for(int n = 0; n < ctvalzn; n++)
			{
				PsevdonimyTextDays[i][h]  = StringReplace(PsevdonimyTextDays[i][h],  valszn[n][0], valszn[n][1], TReplaceFlags() << rfReplaceAll << rfIgnoreCase);
				UslPervBuyTextDays[i][h]  = StringReplace(UslPervBuyTextDays[i][h],  valszn[n][0], valszn[n][1], TReplaceFlags() << rfReplaceAll << rfIgnoreCase);
				UslPervSellTextDays[i][h] = StringReplace(UslPervSellTextDays[i][h], valszn[n][0], valszn[n][1], TReplaceFlags() << rfReplaceAll << rfIgnoreCase);
				UslVstrBuyTextDays[i][h]  = StringReplace(UslVstrBuyTextDays[i][h],  valszn[n][0], valszn[n][1], TReplaceFlags() << rfReplaceAll << rfIgnoreCase);
				UslVstrSellTextDays[i][h] = StringReplace(UslVstrSellTextDays[i][h], valszn[n][0], valszn[n][1], TReplaceFlags() << rfReplaceAll << rfIgnoreCase);
			}
		}
	}

    SetCurUslDayHour();
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::FormDeactivate(TObject *Sender)
{                               
	bool minsz = IsIconic(SmartTest->Handle);
	if(minsz)
	{
		zapret_alpha = true;
        TimerBlendUp->Enabled = true;
    }
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::FormActivate(TObject *Sender)
{
	zapret_alpha = false;
}
//---------------------------------------------------------------------------


void __fastcall TSmartTest::ReSizeConstClick(TObject *Sender)
{
	ReSizeUslClick(ConstVals);
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::ButtonUlutshaizerClick(TObject *Sender)
{
	// Найдем выделенное число
	AnsiString dig = Trim(ConstVals->SelText);
	if(dig == "")
	{
		ShowMessage("Ничего не выделено!");
		return;
	}
	int dg = 0;
	if(!TryStrToInt(dig, dg))
	{
        ShowMessage("Выделенный параметр не является числом!");
		return;
	}
	int selstart  = ConstVals->SelStart;
	int sellength = ConstVals->SelLength;
    ConstVals->SetFocus();

	AnsiString dochisla    = ConstVals->Text.SubString(1, selstart);
	AnsiString poslechisla = ConstVals->Text.SubString(selstart + sellength + 1, ConstVals->Text.Length() - selstart - sellength);

	// Будем двигаться от указанного числа -240 до +240 с шагом 10, минимум 80
	float curMaxRealSummInDay = -1000000;
	int curMaxD = dg;
	for(int i = 0; i < 40; i++)
	{
		int d = dg + (i - 24)*10;
		if(d < 80)
		   continue;

        ConstVals->Text = dochisla + IntToStr(d) + poslechisla;
		ConstVals->SelStart  = selstart;
		ConstVals->SelLength = sellength;

		CurRealSumInDay = 0;
		ButtonMassAnalizeClick(ButtonMassAnalize);
		if(CurRealSumInDay > curMaxRealSummInDay)
		{
			curMaxRealSummInDay = CurRealSumInDay;
			curMaxD = d;
		}
	}

	ConstVals->Text = dochisla + IntToStr(curMaxD) + poslechisla;
    ConstVals->SetFocus();
	ConstVals->SelStart  = selstart;
	ConstVals->SelLength = sellength;
	ButtonMassAnalizeClick(ButtonMassAnalize);
}
//---------------------------------------------------------------------------


void __fastcall TSmartTest::ButUslAllHoursClick(TObject *Sender)
{
	if(!Sender)
       return;

	ButUslAllHours->Font->Style = TFontStyles();
	ButUslH10->Font->Style = TFontStyles();
	ButUslH11->Font->Style = TFontStyles();
	ButUslH12->Font->Style = TFontStyles();
	ButUslH13->Font->Style = TFontStyles();
	ButUslH14->Font->Style = TFontStyles();
	ButUslH15->Font->Style = TFontStyles();
	ButUslH16->Font->Style = TFontStyles();
	ButUslH17->Font->Style = TFontStyles();
	ButUslH18->Font->Style = TFontStyles();
	ButUslH19->Font->Style = TFontStyles();
	ButUslH20->Font->Style = TFontStyles();
	ButUslH21->Font->Style = TFontStyles();
	ButUslH22->Font->Style = TFontStyles();
	ButUslH23->Font->Style = TFontStyles();

	CurUslNumHour = 0;

	if(Sender == ButUslAllHours)
		CurUslNumHour = USL_ALL_HOURS;
	if(Sender == ButUslH10)
		CurUslNumHour = USL_HOUR_10;
	if(Sender == ButUslH11)
		CurUslNumHour = USL_HOUR_11;
	if(Sender == ButUslH12)
		CurUslNumHour = USL_HOUR_12;
	if(Sender == ButUslH13)
		CurUslNumHour = USL_HOUR_13;
	if(Sender == ButUslH14)
		CurUslNumHour = USL_HOUR_14;
	if(Sender == ButUslH15)
		CurUslNumHour = USL_HOUR_15;
	if(Sender == ButUslH16)
		CurUslNumHour = USL_HOUR_16;
	if(Sender == ButUslH17)
		CurUslNumHour = USL_HOUR_17;
	if(Sender == ButUslH18)
		CurUslNumHour = USL_HOUR_18;
	if(Sender == ButUslH19)
		CurUslNumHour = USL_HOUR_19;
	if(Sender == ButUslH20)
		CurUslNumHour = USL_HOUR_20;
	if(Sender == ButUslH21)
		CurUslNumHour = USL_HOUR_21;
	if(Sender == ButUslH22)
		CurUslNumHour = USL_HOUR_22;
	if(Sender == ButUslH23)
		CurUslNumHour = USL_HOUR_23;

	((TSpeedButton*)Sender)->Font->Style = TFontStyles() << fsBold << fsUnderline;
	SetCurUslDayHour();
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::ButMakePodgonUslovClick(TObject *Sender)
{
	if(count_orders <= 0)
	{
		ShowMessage("Нет ни одной сделки.\r\nСделайте массовый анализ или поменяйте условия.");
		return;
	}

	if(taimframe_orders >= 10 || !CurCandles[taimframe_orders] || !CurCountCandles[taimframe_orders])
	{
		ShowMessage("Нет данных по свечкам.");
		return;
	}

	float ct_candles_mpu = 0;
	float strogost_mpu   = 0;
	int smesh_mpu = 0;

	if(!TryStrToFloat(CountCandlesMPU->Text, ct_candles_mpu) ||
	   !TryStrToFloat(StrogostMPU->Text, strogost_mpu) || ct_candles_mpu <= 0 ||
	   !TryStrToInt(SmeshMPU->Text, smesh_mpu) || smesh_mpu > 0)
	{
		ShowMessage("Одно из параметров неверно задано.");
		return;
	}

	int ct_cndls_mpu = 0;
	if(UO->Checked) ct_cndls_mpu++;
	if(UC->Checked) ct_cndls_mpu++;
	if(UL->Checked) ct_cndls_mpu++;
	if(UH->Checked) ct_cndls_mpu++;
	if(UV->Checked) ct_cndls_mpu++;
	if(UD->Checked) ct_cndls_mpu++;
	if(UDg->Checked) ct_cndls_mpu++;
	if(UCC->Checked) ct_cndls_mpu++;
	if(UCC2->Checked) ct_cndls_mpu++;
	if(UD10->Checked) ct_cndls_mpu++;
	if(UDt10->Checked) ct_cndls_mpu++;
	if(UC10C->Checked) ct_cndls_mpu++;
	if(UC10O->Checked) ct_cndls_mpu++;
	if(UDt3->Checked) ct_cndls_mpu++;
	if(UDg3->Checked) ct_cndls_mpu++;
	if(UfD3->Checked) ct_cndls_mpu++;
	if(UDt7->Checked) ct_cndls_mpu++;
	if(UDt11->Checked) ct_cndls_mpu++;

	if(ct_candles_mpu*ct_cndls_mpu*2 > MAX_COUNT_PERV_USLOV)
	{
		ShowMessage("Превышено максимальное кол-во свечек для ограничения.");
		return;
	}

    bool uslovhours = PodgonUslovHours->Checked;

    // Выделим память под пространство ограничений по часам
	struct space_limits* slbuy[15];
	struct space_limits* slsell[15];
	for(int i = 0; i < 15; i++)
	{
		slbuy[i]  = (struct space_limits*)malloc(sizeof(struct space_limits)*ct_candles_mpu);
		slsell[i] = (struct space_limits*)malloc(sizeof(struct space_limits)*ct_candles_mpu);
		if(!slbuy[i] || !slsell[i])
		{
			ShowMessage("Ошибка ввыделения памяти.");
			return;
		}
		ZeroMemory(slbuy[i], sizeof(struct space_limits)*ct_candles_mpu);
		ZeroMemory(slsell[i], sizeof(struct space_limits)*ct_candles_mpu);
		for(int n = 0; n < ct_candles_mpu; n++)
		{
			slbuy[i][n].Init();
			slsell[i][n].Init();
		}
    }

	// Берём только прибыльные сделки
	for(int i = 0; i < count_orders; i++)
	{
		if((orders[i].up && orders[i].price1 - orders[i].price2 >= 0) ||
		   (!orders[i].up && orders[i].price2 - orders[i].price1 >= 0) )
			continue;

		int ind_sl = 0;
        // По часам
		if(uslovhours)
		{
			ind_sl = cur_tmdmy_candles[taimframe_orders][orders[i].id_candle1 - 1].hour - 10;
        }
		struct space_limits* sl = slbuy[ind_sl];
		if(!orders[i].up) sl = slsell[ind_sl];

		int c = orders[i].id_candle1;
		float pr = CurCandles[taimframe_orders][c - 1].priceclose;

		int ch = 0;
		for(int n = 0; n < ct_candles_mpu; n++)
		{
			int ind = c - n - 1 + smesh_mpu;
			if(ind - 10 < 0)
			   continue;

			struct candlesdata* cd  = &CurCandles[taimframe_orders][ind];
			struct candlesdata* cdl = &CurCandles[taimframe_orders][ind - 1];
			struct candlesdata* cd2 = &CurCandles[taimframe_orders][ind - 2];
			struct candlesdata* cd3 = &CurCandles[taimframe_orders][ind - 3];
			struct candlesdata* cd4 = &CurCandles[taimframe_orders][ind - 4];
			struct candlesdata* cd5 = &CurCandles[taimframe_orders][ind - 5];
			struct candlesdata* cd6 = &CurCandles[taimframe_orders][ind - 6];
			struct candlesdata* cd7 = &CurCandles[taimframe_orders][ind - 7];
			struct candlesdata* cd8 = &CurCandles[taimframe_orders][ind - 8];
			struct candlesdata* cd9 = &CurCandles[taimframe_orders][ind - 9];
			struct candlesdata* cd10 = &CurCandles[taimframe_orders][ind - 10];

			if(pr - cd->priceopen < sl[n].min_open_price) sl[n].min_open_price = pr - cd->priceopen;
			if(pr - cd->priceopen > sl[n].max_open_price) sl[n].max_open_price = pr - cd->priceopen;
			sl[n].avg_open_price += pr - cd->priceopen;

			if(pr - cd->priceclose < sl[n].min_close_price) sl[n].min_close_price = pr - cd->priceclose;
			if(pr - cd->priceclose > sl[n].max_close_price) sl[n].max_close_price = pr - cd->priceclose;
			sl[n].avg_close_price += pr - cd->priceclose;

			if(pr - cd->low < sl[n].min_low_price) sl[n].min_low_price = pr - cd->low;
			if(pr - cd->low > sl[n].max_low_price) sl[n].max_low_price = pr - cd->low;
			sl[n].avg_low_price += pr - cd->low;

			if(pr - cd->high < sl[n].min_high_price) sl[n].min_high_price = pr - cd->high;
			if(pr - cd->high > sl[n].max_high_price) sl[n].max_high_price = pr - cd->high;
			sl[n].avg_high_price += pr - cd->high;

			if(cd->volume < sl[n].min_volume) sl[n].min_volume = cd->volume;
			if(cd->volume > sl[n].max_volume) sl[n].max_volume = cd->volume;
			sl[n].avg_volume += cd->volume;

			float delta = cd->priceclose - cd->priceopen;
			if(delta < sl[n].min_delta) sl[n].min_delta = delta;
			if(delta > sl[n].max_delta) sl[n].max_delta = delta;
			sl[n].avg_delta += delta;

			float drag = cd->priceclose - cd->high + cd->priceclose - cd->low;
			if(drag < sl[n].min_drag) sl[n].min_drag = drag;
			if(drag > sl[n].max_drag) sl[n].max_drag = drag;
			sl[n].avg_drag += drag;

			float close_close = cd->priceclose - cdl->priceclose;
			if(close_close < sl[n].min_close_close) sl[n].min_close_close = close_close;
			if(close_close > sl[n].max_close_close) sl[n].max_close_close = close_close;
			sl[n].avg_close_close += close_close;

			float close_close2 = cd->priceclose - cd2->priceclose;
			if(close_close2 < sl[n].min_close_close2) sl[n].min_close_close2 = close_close2;
			if(close_close2 > sl[n].max_close_close2) sl[n].max_close_close2 = close_close2;
			sl[n].avg_close_close2 += close_close2;

			float delta3 = cd->priceclose - cd->priceopen + cdl->priceclose - cdl->priceopen + cd2->priceclose - cd2->priceopen;
			if(delta3 < sl[n].min_delta3) sl[n].min_delta3 = delta3;
			if(delta3 > sl[n].max_delta3) sl[n].max_delta3 = delta3;
			//sl[n].avg_delta3 += delta3;

			float drag3 = cd->priceclose - cd->high + cd->priceclose - cd->low +
						  cdl->priceclose - cdl->high + cdl->priceclose - cdl->low +
						  cd2->priceclose - cd2->high + cd2->priceclose - cd2->low;
			if(drag3 < sl[n].min_drag3) sl[n].min_drag3 = drag3;
			if(drag3 > sl[n].max_drag3) sl[n].max_drag3 = drag3;

			float fdelta3 = (cd->priceclose - cd->priceopen) - (cdl->priceclose - cdl->priceopen) +
							(cdl->priceclose - cdl->priceopen) - (cd2->priceclose - cd2->priceopen) +
							(cd2->priceclose - cd2->priceopen) - (cd3->priceclose - cd3->priceopen);
			if(fdelta3 < sl[n].min_fdelta3) sl[n].min_fdelta3 = fdelta3;
			if(fdelta3 > sl[n].max_fdelta3) sl[n].max_fdelta3 = fdelta3;

			float delta7 = (cd->priceclose - cd->priceopen) + (cdl->priceclose - cdl->priceopen) +
						   (cd2->priceclose - cd2->priceopen) + (cd3->priceclose - cd3->priceopen) +
						   (cd4->priceclose - cd4->priceopen) + (cd5->priceclose - cd5->priceopen) +
						   (cd6->priceclose - cd6->priceopen);
			if(delta7 < sl[n].min_delta7) sl[n].min_delta7 = delta7;
			if(delta7 > sl[n].max_delta7) sl[n].max_delta7 = delta7;

			float delta11 = (cd->priceclose - cd->priceopen) + (cdl->priceclose - cdl->priceopen) +
							(cd2->priceclose - cd2->priceopen) + (cd3->priceclose - cd3->priceopen) +
							(cd4->priceclose - cd4->priceopen) + (cd5->priceclose - cd5->priceopen) +
							(cd6->priceclose - cd6->priceopen) + (cd7->priceclose - cd7->priceopen) +
							(cd8->priceclose - cd8->priceopen) + (cd9->priceclose - cd9->priceopen) +
							(cd10->priceclose - cd10->priceopen);
			if(delta11 < sl[n].min_delta11) sl[n].min_delta11 = delta11;
			if(delta11 > sl[n].max_delta11) sl[n].max_delta11 = delta11;

			ch++;

		}

        if(ch > 0)
		for(int n = 0; n < ct_candles_mpu; n++)
		{
			sl[n].avg_open_price   /= ch;
			sl[n].avg_close_price  /= ch;
			sl[n].avg_low_price    /= ch;
			sl[n].avg_high_price   /= ch;
			sl[n].avg_volume       /= ch;
			sl[n].avg_delta        /= ch;
			sl[n].avg_drag         /= ch;
			sl[n].avg_close_close  /= ch;
			sl[n].avg_close_close2 /= ch;
		}

		// Найдем 10 часовую свечку
		int dom = cur_tmdmy_candles[taimframe_orders][c - 1].dayofmonth;
		for(int cn = c - 1; cn >= 0; cn--)
		{
			if(cur_tmdmy_candles[taimframe_orders][cn].dayofmonth != dom)
			   break;

			if(cur_tmdmy_candles[taimframe_orders][cn].hour == 10 && cur_tmdmy_candles[taimframe_orders][cn].minute == 0)
			{
				struct candlesdata* cd  = &CurCandles[taimframe_orders][cn];
				float delta10 = cd->priceclose - cd->priceopen;
				float D10 = fabs(cd->high - cd->low);
				float C10C = pr - cd->priceclose;
				float C10O = pr - cd->priceopen;

				sl[1].min_delta_10 = sl[0].min_delta_10;
				sl[1].max_delta_10 = sl[0].max_delta_10;
				if(delta10 < sl[0].min_delta_10) sl[0].min_delta_10 = delta10;
				if(delta10 > sl[0].max_delta_10) sl[0].max_delta_10 = delta10;

				sl[1].min_D_10 = sl[0].min_D_10;
				sl[1].max_D_10 = sl[0].max_D_10;
				if(D10 < sl[0].min_D_10) sl[0].min_D_10 = D10;
				if(D10 > sl[0].max_D_10) sl[0].max_D_10 = D10;

				if(C10C < sl[0].min_close_10_close) sl[0].min_close_10_close = C10C;
				if(C10C > sl[0].max_close_10_close) sl[0].max_close_10_close = C10C;

				if(C10O < sl[0].min_close_10_open) sl[0].min_close_10_open = C10O;
				if(C10O > sl[0].max_close_10_open) sl[0].max_close_10_open = C10O;

				break;
			}
		}

	}

	// Создадим ограничивающие условия по часам
	AnsiString uslov_buy  = "";
	AnsiString uslov_sell = "";
	AnsiString tf = IntToStr(taimframe_orders);
	AnsiString dlm = ",";
	AnsiString cltf = "close[" + tf + "] -";
	int ct_hours = 1;
	if(uslovhours)
	   ct_hours = 14;
    int h = 0;
	for(int i = 0; i < ct_candles_mpu; i++)
	{

		AnsiString n  = IntToStr(-i + smesh_mpu);
		AnsiString n2 = IntToStr(-i + smesh_mpu - 1);
		AnsiString n3 = IntToStr(-i + smesh_mpu - 2);
		AnsiString tfn  = "[" + tf + "][" + n + "]";
		AnsiString tfn2 = "[" + tf + "; 0][" + n + "]";
		AnsiString tfn3 = "[" + tf + "][" + n2 + "]";
		AnsiString tfn4 = "[" + tf + "][" + n3 + "]";

		if(UO->Checked)
		{
				if(slbuy[h][i].min_open_price != 10000000)
				   uslov_buy += cltf + " open" + tfn + " >= " + FloatToStr(slbuy[h][i].min_open_price - strogost_mpu) + " " + dlm + " \r\n";
				if(slbuy[h][i].max_open_price != -10000000)
				   uslov_buy += cltf + " open" + tfn + " <= " + FloatToStr(slbuy[h][i].max_open_price + strogost_mpu) + " " + dlm + " \r\n";

				if(slsell[h][i].min_open_price != 10000000)
				   uslov_sell += cltf + " open" + tfn + " >= " + FloatToStr(slsell[h][i].min_open_price - strogost_mpu) + " " + dlm + " \r\n";
				if(slsell[h][i].max_open_price != -10000000)
				   uslov_sell += cltf + " open" + tfn + " <= " + FloatToStr(slsell[h][i].max_open_price + strogost_mpu) + " " + dlm + " \r\n";
		}


			if(UC->Checked)
			{
				if(slbuy[h][i].min_close_price != 10000000)
				   uslov_buy += cltf + " close" + tfn + " >= " + FloatToStr(slbuy[h][i].min_close_price - strogost_mpu) + " " + dlm + " \r\n";
				if(slbuy[h][i].max_close_price != -10000000)
				   uslov_buy += cltf + " close" + tfn + " <= " + FloatToStr(slbuy[h][i].max_close_price + strogost_mpu) + " " + dlm + " \r\n";

				if(slsell[h][i].min_close_price != 10000000)
				   uslov_sell += cltf + " close" + tfn + " >= " + FloatToStr(slsell[h][i].min_close_price - strogost_mpu) + " " + dlm + " \r\n";
				if(slsell[h][i].max_close_price != -10000000)
				   uslov_sell += cltf + " close" + tfn + " <= " + FloatToStr(slsell[h][i].max_close_price + strogost_mpu) + " " + dlm + " \r\n";
			}

			if(UH->Checked)
			{
				if(slbuy[h][i].min_high_price != 10000000)
				   uslov_buy += cltf + " high" + tfn + " >= " + FloatToStr(slbuy[h][i].min_high_price - strogost_mpu) + " " + dlm + " \r\n";
				if(slbuy[h][i].max_high_price != -10000000)
				   uslov_buy += cltf + " high" + tfn + " <= " + FloatToStr(slbuy[h][i].max_high_price + strogost_mpu) + " " + dlm + " \r\n";
				//uslov_buy += cltf + " high" + tfn + " <= " + FloatToStr(slbuy[h][i].avg_high_price + strogost_mpu) + " " + dlm + " \r\n";

				if(slsell[h][i].min_high_price != 10000000)
				   uslov_sell += cltf + " high" + tfn + " >= " + FloatToStr(slsell[h][i].min_high_price - strogost_mpu) + " " + dlm + " \r\n";
				if(slsell[h][i].max_high_price != -10000000)
				   uslov_sell += cltf + " high" + tfn + " <= " + FloatToStr(slsell[h][i].max_high_price + strogost_mpu) + " " + dlm + " \r\n";
				//uslov_sell += cltf + " high" + tfn + " <= " + FloatToStr(slsell[h][i].avg_high_price + strogost_mpu) + " " + dlm + " \r\n";
			}

			if(UL->Checked)
			{
				if(slbuy[h][i].min_low_price != 10000000)
				   uslov_buy += cltf + " low" + tfn + " >= " + FloatToStr(slbuy[h][i].min_low_price - strogost_mpu) + " " + dlm + " \r\n";
				if(slbuy[h][i].max_low_price != -10000000)
				   uslov_buy += cltf + " low" + tfn + " <= " + FloatToStr(slbuy[h][i].max_low_price + strogost_mpu) + " " + dlm + " \r\n";
				//uslov_buy += cltf + " low" + tfn + " >= " + FloatToStr(slbuy[h][i].avg_low_price - strogost_mpu) + " " + dlm + " \r\n";

				if(slsell[h][i].min_low_price != 10000000)
				   uslov_sell += cltf + " low" + tfn + " >= " + FloatToStr(slsell[h][i].min_low_price - strogost_mpu) + " " + dlm + " \r\n";
				if(slsell[h][i].max_low_price != -10000000)
				   uslov_sell += cltf + " low" + tfn + " <= " + FloatToStr(slsell[h][i].max_low_price + strogost_mpu) + " " + dlm + " \r\n";
				//uslov_sell += cltf + " low" + tfn + " >= " + FloatToStr(slsell[h][i].avg_low_price - strogost_mpu) + " " + dlm + " \r\n";
			}

			if(UV->Checked)
			{
				if(slbuy[h][i].min_volume != 10000000)
				   uslov_buy += "volume" + tfn + " >= " + FloatToStr(slbuy[h][i].min_volume - strogost_mpu) + " " + dlm + " \r\n";
				if(slbuy[h][i].max_volume != -10000000)
				   uslov_buy += "volume" + tfn + " <= " + FloatToStr(slbuy[h][i].max_volume + strogost_mpu) + " " + dlm + " \r\n";

				if(slsell[h][i].min_volume != 10000000)
				   uslov_sell += "volume" + tfn + " >= " + FloatToStr(slsell[h][i].min_volume - strogost_mpu) + " " + dlm + " \r\n";
				if(slsell[h][i].max_volume != -10000000)
				   uslov_sell += "volume" + tfn + " <= " + FloatToStr(slsell[h][i].max_volume + strogost_mpu) + " " + dlm + " \r\n";
			}

			if(UD->Checked)
			{
				if(slbuy[h][i].min_delta != 10000000)
				   uslov_buy += "close" + tfn + " - open" + tfn + " >= " + FloatToStr(slbuy[h][i].min_delta - strogost_mpu) + " " + dlm + " \r\n";
				if(slbuy[h][i].max_delta != -10000000)
				   uslov_buy += "close" + tfn + " - open" + tfn + " <= " + FloatToStr(slbuy[h][i].max_delta + strogost_mpu) + " " + dlm + " \r\n";

				if(slsell[h][i].min_delta != 10000000)
				   uslov_sell += "close" + tfn + " - open" + tfn + " >= " + FloatToStr(slsell[h][i].min_delta - strogost_mpu) + " " + dlm + " \r\n";
				if(slsell[h][i].max_delta != -10000000)
				   uslov_sell += "close" + tfn + " - open" + tfn + " <= " + FloatToStr(slsell[h][i].max_delta + strogost_mpu) + " " + dlm + " \r\n";
			}

			if(UDg->Checked)
			{
				if(slbuy[h][i].min_drag != 10000000)
				   uslov_buy += "drag" + tfn2 + " >= " + FloatToStr(slbuy[h][i].min_drag - strogost_mpu) + " " + dlm + " \r\n";
				if(slbuy[h][i].max_drag != -10000000)
				   uslov_buy += "drag" + tfn2 + " <= " + FloatToStr(slbuy[h][i].max_drag + strogost_mpu) + " " + dlm + " \r\n";

				if(slsell[h][i].min_drag != 10000000)
				   uslov_sell += "drag" + tfn2 + " >= " + FloatToStr(slsell[h][i].min_drag - strogost_mpu) + " " + dlm + " \r\n";
				if(slsell[h][i].max_drag != -10000000)
				   uslov_sell += "drag" + tfn2 + " <= " + FloatToStr(slsell[h][i].max_drag + strogost_mpu) + " " + dlm + " \r\n";
			}

			if(UCC->Checked)
			{
				if(slbuy[h][i].min_close_close != 10000000)
				   uslov_buy += "close" + tfn + " - close" + tfn3 + " >= " + FloatToStr(slbuy[h][i].min_close_close - strogost_mpu) + " " + dlm + " \r\n";
				if(slbuy[h][i].max_close_close != -10000000)
				   uslov_buy += "close" + tfn + " - close" + tfn3 + " <= " + FloatToStr(slbuy[h][i].max_close_close + strogost_mpu) + " " + dlm + " \r\n";

				if(slsell[h][i].min_close_close != 10000000)
				   uslov_sell += "close" + tfn + " - close" + tfn3 + " >= " + FloatToStr(slsell[h][i].min_close_close - strogost_mpu) + " " + dlm + " \r\n";
				if(slsell[h][i].max_close_close != -10000000)
				   uslov_sell += "close" + tfn + " - close" + tfn3 + " <= " + FloatToStr(slsell[h][i].max_close_close + strogost_mpu) + " " + dlm + " \r\n";
			}

			if(UCC2->Checked)
			{
				if(slbuy[h][i].min_close_close2 != 10000000)
				   uslov_buy += "close" + tfn + " - close" + tfn4 + " >= " + FloatToStr(slbuy[h][i].min_close_close2 - strogost_mpu) + " " + dlm + " \r\n";
				if(slbuy[h][i].max_close_close2 != -10000000)
				   uslov_buy += "close" + tfn + " - close" + tfn4 + " <= " + FloatToStr(slbuy[h][i].max_close_close2 + strogost_mpu) + " " + dlm + " \r\n";

				if(slsell[h][i].min_close_close2 != 10000000)
				   uslov_sell += "close" + tfn + " - close" + tfn4 + " >= " + FloatToStr(slsell[h][i].min_close_close2 - strogost_mpu) + " " + dlm + " \r\n";
				if(slsell[h][i].max_close_close2 != -10000000)
				   uslov_sell += "close" + tfn + " - close" + tfn4 + " <= " + FloatToStr(slsell[h][i].max_close_close2 + strogost_mpu) + " " + dlm + " \r\n";
			}

			if(UDt3->Checked)
			{
				AnsiString dlt = "delta[" + tf + "; 3][" + n + "]";

				if(slbuy[h][i].min_delta3 != 10000000)
				   uslov_buy += dlt + " >= " + FloatToStr(slbuy[h][i].min_delta3 - strogost_mpu) + " " + dlm + " \r\n";
				if(slbuy[h][i].max_delta3 != -10000000)
				   uslov_buy += dlt + " <= " + FloatToStr(slbuy[h][i].max_delta3 + strogost_mpu) + " " + dlm + " \r\n";

				if(slsell[h][i].min_delta3 != 10000000)
				   uslov_sell += dlt + " >= " + FloatToStr(slsell[h][i].min_delta3 - strogost_mpu) + " " + dlm + " \r\n";
				if(slsell[h][i].max_delta3 != -10000000)
				   uslov_sell += dlt + " <= " + FloatToStr(slsell[h][i].max_delta3 + strogost_mpu) + " " + dlm + " \r\n";
			}

			if(UDg3->Checked)
			{
				AnsiString dlt = "realdrag[" + tf + "; 3][" + n + "]";

				if(slbuy[h][i].min_drag3 != 10000000)
				   uslov_buy += dlt + " >= " + FloatToStr(slbuy[h][i].min_drag3 - strogost_mpu) + " " + dlm + " \r\n";
				if(slbuy[h][i].max_drag3 != -10000000)
				   uslov_buy += dlt + " <= " + FloatToStr(slbuy[h][i].max_drag3 + strogost_mpu) + " " + dlm + " \r\n";

				if(slsell[h][i].min_drag3 != 10000000)
				   uslov_sell += dlt + " >= " + FloatToStr(slsell[h][i].min_drag3 - strogost_mpu) + " " + dlm + " \r\n";
				if(slsell[h][i].max_drag3 != -10000000)
				   uslov_sell += dlt + " <= " + FloatToStr(slsell[h][i].max_drag3 + strogost_mpu) + " " + dlm + " \r\n";
			}

			if(UfD3->Checked)
			{
				AnsiString dlt = "fdelta[" + tf + "; 3][" + n + "]";

				if(slbuy[h][i].min_fdelta3 != 10000000)
				   uslov_buy += dlt + " >= " + FloatToStr(slbuy[h][i].min_fdelta3 - strogost_mpu) + " " + dlm + " \r\n";
				if(slbuy[h][i].max_fdelta3 != -10000000)
				   uslov_buy += dlt + " <= " + FloatToStr(slbuy[h][i].max_fdelta3 + strogost_mpu) + " " + dlm + " \r\n";

				if(slsell[h][i].min_fdelta3 != 10000000)
				   uslov_sell += dlt + " >= " + FloatToStr(slsell[h][i].min_fdelta3 - strogost_mpu) + " " + dlm + " \r\n";
				if(slsell[h][i].max_fdelta3 != -10000000)
				   uslov_sell += dlt + " <= " + FloatToStr(slsell[h][i].max_fdelta3 + strogost_mpu) + " " + dlm + " \r\n";
			}

			if(UDt7->Checked)
			{
				AnsiString dlt = "delta[" + tf + "; 7][" + n + "]";

				if(slbuy[h][i].min_delta7 != 10000000)
				   uslov_buy += dlt + " >= " + FloatToStr(slbuy[h][i].min_delta7 - strogost_mpu) + " " + dlm + " \r\n";
				if(slbuy[h][i].max_delta7 != -10000000)
				   uslov_buy += dlt + " <= " + FloatToStr(slbuy[h][i].max_delta7 + strogost_mpu) + " " + dlm + " \r\n";

				if(slsell[h][i].min_delta7 != 10000000)
				   uslov_sell += dlt + " >= " + FloatToStr(slsell[h][i].min_delta7 - strogost_mpu) + " " + dlm + " \r\n";
				if(slsell[h][i].max_delta7 != -10000000)
				   uslov_sell += dlt + " <= " + FloatToStr(slsell[h][i].max_delta7 + strogost_mpu) + " " + dlm + " \r\n";
			}

			if(UDt11->Checked)
			{
				AnsiString dlt = "delta[" + tf + "; 11][" + n + "]";

				if(slbuy[h][i].min_delta11 != 10000000)
				   uslov_buy += dlt + " >= " + FloatToStr(slbuy[h][i].min_delta11 - strogost_mpu) + " " + dlm + " \r\n";
				if(slbuy[h][i].max_delta11 != -10000000)
				   uslov_buy += dlt + " <= " + FloatToStr(slbuy[h][i].max_delta11 + strogost_mpu) + " " + dlm + " \r\n";

				if(slsell[h][i].min_delta11 != 10000000)
				   uslov_sell += dlt + " >= " + FloatToStr(slsell[h][i].min_delta11 - strogost_mpu) + " " + dlm + " \r\n";
				if(slsell[h][i].max_delta11 != -10000000)
				   uslov_sell += dlt + " <= " + FloatToStr(slsell[h][i].max_delta11 + strogost_mpu) + " " + dlm + " \r\n";
			}
		}

		if(UD10->Checked)
		{
			if(slbuy[h][0].min_D_10 != 10000000)
				uslov_buy += "pvtime[" + tf + "; 10:00; High] - pvtime[" + tf + "; 10:00; Low] >= " + FloatToStr(slbuy[h][0].min_D_10 - strogost_mpu) + " " + dlm + " \r\n";
			if(slbuy[h][0].max_D_10 != -10000000)
				uslov_buy += "pvtime[" + tf + "; 10:00; High] - pvtime[" + tf + "; 10:00; Low] <= " + FloatToStr(slbuy[h][0].max_D_10 + strogost_mpu) + " " + dlm + " \r\n";

			if(slsell[h][0].min_D_10 != 10000000)
				uslov_sell += "pvtime[" + tf + "; 10:00; High] - pvtime[" + tf + "; 10:00; Low] >= " + FloatToStr(slsell[h][0].min_D_10 - strogost_mpu) + " " + dlm + " \r\n";
			if(slsell[h][0].max_D_10 != -10000000)
				uslov_sell += "pvtime[" + tf + "; 10:00; High] - pvtime[" + tf + "; 10:00; Low] <= " + FloatToStr(slsell[h][0].max_D_10 + strogost_mpu) + " " + dlm + " \r\n";
		}

		if(UDt10->Checked)
		{
			if(slbuy[h][0].min_delta_10 != 10000000)
				uslov_buy += "pvtime[" + tf + "; 10:00; Close] - pvtime[" + tf + "; 10:00; Open] >= " + FloatToStr(slbuy[h][0].min_delta_10 - strogost_mpu) + " " + dlm + " \r\n";
			if(slbuy[h][0].max_delta_10 != -10000000)
				uslov_buy += "pvtime[" + tf + "; 10:00; Close] - pvtime[" + tf + "; 10:00; Open] <= " + FloatToStr(slbuy[h][0].max_delta_10 + strogost_mpu) + " " + dlm + " \r\n";

			if(slsell[h][0].min_delta_10 != 10000000)
				uslov_sell += "pvtime[" + tf + "; 10:00; Close] - pvtime[" + tf + "; 10:00; Open] >= " + FloatToStr(slsell[h][0].min_delta_10 - strogost_mpu) + " " + dlm + " \r\n";
			if(slsell[h][0].max_delta_10 != -10000000)
				uslov_sell += "pvtime[" + tf + "; 10:00; Close] - pvtime[" + tf + "; 10:00; Open] <= " + FloatToStr(slsell[h][0].max_delta_10 + strogost_mpu) + " " + dlm + " \r\n";
		}

		if(UC10C->Checked)
		{
			if(slbuy[h][0].min_close_10_close != 10000000)
				uslov_buy += cltf + " pvtime[" + tf + "; 10:00; Close] >= " + FloatToStr(slbuy[h][0].min_close_10_close - strogost_mpu) + " " + dlm + " \r\n";
			if(slbuy[h][0].max_close_10_close != -10000000)
				uslov_buy += cltf + " pvtime[" + tf + "; 10:00; Close] <= " + FloatToStr(slbuy[h][0].max_close_10_close + strogost_mpu) + " " + dlm + " \r\n";

			if(slsell[h][0].min_close_10_close != 10000000)
				uslov_sell += cltf + " pvtime[" + tf + "; 10:00; Close] >= " + FloatToStr(slsell[h][0].min_close_10_close - strogost_mpu) + " " + dlm + " \r\n";
			if(slsell[h][0].max_close_10_close != -10000000)
				uslov_sell += cltf + " pvtime[" + tf + "; 10:00; Close] <= " + FloatToStr(slsell[h][0].max_close_10_close + strogost_mpu) + " " + dlm + " \r\n";
		}

		if(UC10O->Checked)
		{
			if(slbuy[h][0].min_close_10_open != 10000000)
				uslov_buy += cltf + " pvtime[" + tf + "; 10:00; Open] >= " + FloatToStr(slbuy[h][0].min_close_10_open - strogost_mpu) + " " + dlm + " \r\n";
			if(slbuy[h][0].max_close_10_open != -10000000)
				uslov_buy += cltf + " pvtime[" + tf + "; 10:00; Open] <= " + FloatToStr(slbuy[h][0].max_close_10_open + strogost_mpu) + " " + dlm + " \r\n";

			if(slsell[h][0].min_close_10_open != 10000000)
				uslov_sell += cltf + " pvtime[" + tf + "; 10:00; Open] >= " + FloatToStr(slsell[h][0].min_close_10_open - strogost_mpu) + " " + dlm + " \r\n";
			if(slsell[h][0].max_close_10_open != -10000000)
				uslov_sell += cltf + " pvtime[" + tf + "; 10:00; Open] <= " + FloatToStr(slsell[h][0].max_close_10_open + strogost_mpu) + " " + dlm + " \r\n";
		}


	ResUsl->Text     = uslov_buy;
	ResUslSell->Text = uslov_sell;

	// Выведем результат этих условий на этих же сделках
	int ct_prib = 0;
	int ct_ubyt = 0;
	for(int i = 0; i < count_orders; i++)
	{
		bool prib = true;
		if((orders[i].up && orders[i].price1 - orders[i].price2 > 0) ||
		   (!orders[i].up && orders[i].price2 - orders[i].price1 > 0) )
			prib = false;

		struct space_limits* sl = slbuy[0];
		if(!orders[i].up) sl = slsell[0];
		if(!sl)
           continue;

		int c = orders[i].id_candle1;
		float pr = CurCandles[taimframe_orders][c - 1].priceclose;

		bool est_prib = true;
		bool est_ubyt = true;
		int ch = 0;
		for(int n = 0; n < ct_candles_mpu; n++)
		{
			int ind = c - n - 1 + smesh_mpu;
			if(ind - 10 < 0)
			   continue;

			struct candlesdata* cd  = &CurCandles[taimframe_orders][ind];
			struct candlesdata* cdl = &CurCandles[taimframe_orders][ind - 1];
			struct candlesdata* cd2 = &CurCandles[taimframe_orders][ind - 2];
			struct candlesdata* cd3 = &CurCandles[taimframe_orders][ind - 3];
			struct candlesdata* cd4 = &CurCandles[taimframe_orders][ind - 4];
			struct candlesdata* cd5 = &CurCandles[taimframe_orders][ind - 5];
			struct candlesdata* cd6 = &CurCandles[taimframe_orders][ind - 6];
			struct candlesdata* cd7 = &CurCandles[taimframe_orders][ind - 7];
			struct candlesdata* cd8 = &CurCandles[taimframe_orders][ind - 8];
			struct candlesdata* cd9 = &CurCandles[taimframe_orders][ind - 9];
			struct candlesdata* cd10 = &CurCandles[taimframe_orders][ind - 10];

			if(UO->Checked && prib && !(pr - cd->priceopen >= sl[n].min_open_price - strogost_mpu && pr - cd->priceopen <= sl[n].max_open_price + strogost_mpu))
			{
				est_prib = false;
				break;
			}
			if(UO->Checked && !prib && !(pr - cd->priceopen >= sl[n].min_open_price - strogost_mpu && pr - cd->priceopen <= sl[n].max_open_price + strogost_mpu))
			{
				est_ubyt = false;
				break;
			}

			if(UC->Checked && prib && !(pr - cd->priceclose >= sl[n].min_close_price - strogost_mpu && pr - cd->priceclose <= sl[n].max_close_price + strogost_mpu))
			{
				est_prib = false;
				break;
			}
			if(UC->Checked && !prib && !(pr - cd->priceclose >= sl[n].min_close_price - strogost_mpu && pr - cd->priceclose <= sl[n].max_close_price + strogost_mpu))
			{
				est_ubyt = false;
				break;
			}

			if(UL->Checked && prib && !(pr - cd->low >= sl[n].min_low_price - strogost_mpu && pr - cd->low <= sl[n].max_low_price + strogost_mpu))
			{
				est_prib = false;
				break;
			}
			if(UL->Checked && !prib && !(pr - cd->low >= sl[n].min_low_price - strogost_mpu && pr - cd->low <= sl[n].max_low_price + strogost_mpu))
			{
				est_ubyt = false;
				break;
			}

			if(UH->Checked && prib && !(pr - cd->high >= sl[n].min_high_price - strogost_mpu && pr - cd->high <= sl[n].max_high_price + strogost_mpu))
			{
				est_prib = false;
				break;
			}
			if(UH->Checked && !prib && !(pr - cd->high >= sl[n].min_high_price - strogost_mpu && pr - cd->high <= sl[n].max_high_price + strogost_mpu))
			{
				est_ubyt = false;
				break;
			}

			if(UV->Checked && prib && !(cd->volume >= sl[n].min_volume - strogost_mpu && cd->volume <= sl[n].max_volume + strogost_mpu))
			{
				est_prib = false;
				break;
			}
			if(UV->Checked && !prib && !(cd->volume >= sl[n].min_volume - strogost_mpu && cd->volume <= sl[n].max_volume + strogost_mpu))
			{
				est_ubyt = false;
				break;
			}

			float delta = cd->priceclose - cd->priceopen;
			if(UD->Checked && prib && !(delta >= sl[n].min_delta - strogost_mpu && delta <= sl[n].max_delta + strogost_mpu))
			{
				est_prib = false;
				break;
			}
			if(UD->Checked && !prib && !(delta >= sl[n].min_delta - strogost_mpu && delta <= sl[n].max_delta + strogost_mpu))
			{
				est_ubyt = false;
				break;
			}

			float drag = cd->priceclose - cd->high + cd->priceclose - cd->low;
			if(UDg->Checked && prib && !(drag >= sl[n].min_drag - strogost_mpu && drag <= sl[n].max_drag + strogost_mpu))
			{
				est_prib = false;
				break;
			}
			if(UDg->Checked && !prib && !(drag >= sl[n].min_drag - strogost_mpu && drag <= sl[n].max_drag + strogost_mpu))
			{
				est_ubyt = false;
				break;
			}

			float close_close = cd->priceclose - cdl->priceclose;
			if(UCC->Checked && prib && !(close_close >= sl[n].min_close_close - strogost_mpu && close_close <= sl[n].max_close_close + strogost_mpu))
			{
				est_prib = false;
				break;
			}
			if(UCC->Checked && !prib && !(close_close >= sl[n].min_close_close - strogost_mpu && close_close <= sl[n].max_close_close + strogost_mpu))
			{
				est_ubyt = false;
				break;
			}

			float close_close2 = cd->priceclose - cd2->priceclose;
			if(UCC2->Checked && prib && !(close_close2 >= sl[n].min_close_close2 - strogost_mpu && close_close2 <= sl[n].max_close_close2 + strogost_mpu))
			{
				est_prib = false;
				break;
			}
			if(UCC2->Checked && !prib && !(close_close2 >= sl[n].min_close_close2 - strogost_mpu && close_close2 <= sl[n].max_close_close2 + strogost_mpu))
			{
				est_ubyt = false;
				break;
			}

			float delta3 = cd->priceclose - cd->priceopen + cdl->priceclose - cdl->priceopen + cd2->priceclose - cd2->priceopen;
			if(UDt3->Checked && prib && !(delta3 >= sl[n].min_delta3 - strogost_mpu && delta3 <= sl[n].max_delta3 + strogost_mpu))
			{
				est_prib = false;
				break;
			}
			if(UDt3->Checked && !prib && !(delta3 >= sl[n].min_delta3 - strogost_mpu && delta3 <= sl[n].max_delta3 + strogost_mpu))
			{
				est_ubyt = false;
				break;
			}

			float drag3 = cd->priceclose - cd->high + cd->priceclose - cd->low +
						  cdl->priceclose - cdl->high + cdl->priceclose - cdl->low +
						  cd2->priceclose - cd2->high + cd2->priceclose - cd2->low;
			if(UDg3->Checked && prib && !(drag3 >= sl[n].min_drag3 - strogost_mpu && drag3 <= sl[n].max_drag3 + strogost_mpu))
			{
				est_prib = false;
				break;
			}
			if(UDg3->Checked && !prib && !(drag3 >= sl[n].min_drag3 - strogost_mpu && drag3 <= sl[n].max_drag3 + strogost_mpu))
			{
				est_ubyt = false;
				break;
			}

			float fdelta3 = (cd->priceclose - cd->priceopen) - (cdl->priceclose - cdl->priceopen) +
							(cdl->priceclose - cdl->priceopen) - (cd2->priceclose - cd2->priceopen) +
							(cd2->priceclose - cd2->priceopen) - (cd3->priceclose - cd3->priceopen);
			if(UfD3->Checked && prib && !(fdelta3 >= sl[n].min_fdelta3 - strogost_mpu && fdelta3 <= sl[n].max_fdelta3 + strogost_mpu))
			{
				est_prib = false;
				break;
			}
			if(UfD3->Checked && !prib && !(fdelta3 >= sl[n].min_fdelta3 - strogost_mpu && fdelta3 <= sl[n].max_fdelta3 + strogost_mpu))
			{
				est_ubyt = false;
				break;
			}

            float delta7 = (cd->priceclose - cd->priceopen) + (cdl->priceclose - cdl->priceopen) +
						   (cd2->priceclose - cd2->priceopen) + (cd3->priceclose - cd3->priceopen) +
						   (cd4->priceclose - cd4->priceopen) + (cd5->priceclose - cd5->priceopen) +
						   (cd6->priceclose - cd6->priceopen);
			if(UDt7->Checked && prib && !(delta7 >= sl[n].min_delta7 - strogost_mpu && delta7 <= sl[n].max_delta7 + strogost_mpu))
			{
				est_prib = false;
				break;
			}
			if(UDt7->Checked && !prib && !(delta7 >= sl[n].min_delta7 - strogost_mpu && delta7 <= sl[n].max_delta7 + strogost_mpu))
			{
				est_ubyt = false;
				break;
			}

			float delta11 = (cd->priceclose - cd->priceopen) + (cdl->priceclose - cdl->priceopen) +
							(cd2->priceclose - cd2->priceopen) + (cd3->priceclose - cd3->priceopen) +
							(cd4->priceclose - cd4->priceopen) + (cd5->priceclose - cd5->priceopen) +
							(cd6->priceclose - cd6->priceopen) + (cd7->priceclose - cd7->priceopen) +
							(cd8->priceclose - cd8->priceopen) + (cd9->priceclose - cd9->priceopen) +
							(cd10->priceclose - cd10->priceopen);
			if(UDt11->Checked && prib && !(delta11 >= sl[n].min_delta11 - strogost_mpu && delta11 <= sl[n].max_delta11 + strogost_mpu))
			{
				est_prib = false;
				break;
			}
			if(UDt11->Checked && !prib && !(delta11 >= sl[n].min_delta11 - strogost_mpu && delta11 <= sl[n].max_delta11 + strogost_mpu))
			{
				est_ubyt = false;
				break;
			}
		}

		// Найдем 10 часовую свечку
		float delta10 = 0;
		float D10     = 0;
		float C10C    = 0;
		float C10O    = 0;
		int dom = cur_tmdmy_candles[taimframe_orders][c - 1].dayofmonth;
		for(int cn = c - 1; cn >= 0; cn--)
		{
			if(cur_tmdmy_candles[taimframe_orders][cn].dayofmonth != dom)
			   break;

			if(cur_tmdmy_candles[taimframe_orders][cn].hour == 10 && cur_tmdmy_candles[taimframe_orders][cn].minute == 0)
			{
				struct candlesdata* cd  = &CurCandles[taimframe_orders][cn];
				delta10 = cd->priceclose - cd->priceopen;
				D10 = fabs(cd->high - cd->low);
				C10C = pr - cd->priceclose;
				C10O = pr - cd->priceopen;
				break;
			}
		}

		if(UD10->Checked && delta10 != 0 && prib && !(delta10 >= sl[0].min_delta_10 - strogost_mpu && delta10 <= sl[0].max_delta_10 + strogost_mpu))
			est_prib = false;
		if(UD10->Checked && delta10 != 0 && !prib && !(delta10 >= sl[0].min_delta_10 - strogost_mpu && delta10 <= sl[0].max_delta_10 + strogost_mpu))
			est_ubyt = false;

		if(UDt10->Checked && D10 != 0 && prib && !(D10 >= sl[0].min_D_10 - strogost_mpu && D10 <= sl[0].max_D_10 + strogost_mpu))
			est_prib = false;
		if(UDt10->Checked && D10 != 0 && !prib && !(D10 >= sl[0].min_D_10 - strogost_mpu && D10 <= sl[0].max_D_10 + strogost_mpu))
			est_ubyt = false;

		if(UC10C->Checked && C10C != 0 && prib && !(C10C >= sl[0].min_close_10_close - strogost_mpu && C10C <= sl[0].max_close_10_close + strogost_mpu))
			est_prib = false;
		if(UC10C->Checked && C10C != 0 && !prib && !(C10C >= sl[0].min_close_10_close - strogost_mpu && C10C <= sl[0].max_close_10_close + strogost_mpu))
			est_ubyt = false;

		if(UC10O->Checked && C10O != 0 && prib && !(C10O >= sl[0].min_close_10_open - strogost_mpu && C10O <= sl[0].max_close_10_open + strogost_mpu))
			est_prib = false;
		if(UC10O->Checked && C10O != 0 && !prib && !(C10O >= sl[0].min_close_10_open - strogost_mpu && C10O <= sl[0].max_close_10_open + strogost_mpu))
			est_ubyt = false;

		if(prib && est_prib)
		   ct_prib++;
		if(!prib && est_ubyt)
		   ct_ubyt++;
	}

	if(slbuy)  free(slbuy);
	if(slsell) free(slsell);

    ResOrdersPodgon->Text = "Приб-ных.=" + IntToStr(ct_prib) + ", убыт-ных=" + IntToStr(ct_ubyt);
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::Button5Click(TObject *Sender)
{
	AnsiString tagst  = "/*[bester]*/";
	AnsiString tagend = "/*[/bester]*/";
	int posst = UslPervBuy->Text.Pos(tagst);
	if(posst > 0)
	{
		int posend = UslPervBuy->Text.Pos(tagend);
		if(posend > 0 && posend > posst)
		{
			UslPervBuy->Text = UslPervBuy->Text.SubString(1, posst + tagst.Length()) + "\r\n" + ResUsl->Text +
							   UslPervBuy->Text.SubString(posend, UslPervBuy->Text.Length() - (posend) + 1 );

			return;
        }
    }

	UslPervBuy->Text = UslPervBuy->Text + ",\r\n /*[bester]*/\r\n" + ResUsl->Text + "\r\n/*[/bester]*/\r\n";
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::Button8Click(TObject *Sender)
{
	AnsiString tagst  = "/*[bester]*/";
	AnsiString tagend = "/*[/bester]*/";
	int posst = UslPervSell->Text.Pos(tagst);
	if(posst > 0)
	{
		int posend = UslPervSell->Text.Pos(tagend);
		if(posend > 0 && posend > posst)
		{
			UslPervSell->Text = UslPervSell->Text.SubString(1, posst + tagst.Length()) + "\r\n" + ResUslSell->Text +
								UslPervSell->Text.SubString(posend, UslPervSell->Text.Length() - (posend) + 1  );

			return;
        }
    }

	UslPervSell->Text = UslPervSell->Text + ",\r\n /*[bester]*/\r\n" + ResUslSell->Text + "\r\n/*[/bester]*/\r\n";
}
//---------------------------------------------------------------------------

void __fastcall TSmartTest::Button9Click(TObject *Sender)
{
	int PeriodSMA = StrToInt(OUEPeriodSMA->Text);
	int CTCandles = StrToInt(OUECTCandles->Text);
	float MinTP   = StrToFloat(OUEMinTP->Text);
	float MaxTP   = StrToFloat(OUEMaxTP->Text);
	float StepTP  = StrToFloat(OUEStepTP->Text);
	int TM        = OUETM->ItemIndex;

	CurCountCandles = FMain->count_candles;
	for(int i = 0; i < 10; i++)
	   CurCandles[i] = (FMain->candles[i]);
	for(int i = 0; i < 10; i++)
	   cur_tmdmy_candles[i] = (FMain->tmdmy_candles[i]);

	if(TM < 0 || TM >= 10 || !CurCandles[TM] || !CurCountCandles[TM])
	{
		ShowMessage("Нет данных по свечкам.");
		return;
	}

	if(PeriodSMA <= 0 || CTCandles < 1 || MinTP <= 0 || MaxTP <= 0 || StepTP < 0)
	{
		ShowMessage("Одно из параметров неверно задано.");
		return;
	}

	struct candlesdata* candles = CurCandles[TM];
	struct timedaymonthyear_candles* tmdmy = cur_tmdmy_candles[TM];

	// Ограничения прри покупке и при продаже
	float* Delta3MinBuy  = (float*)malloc(sizeof(float)*CTCandles);
	float* Delta3MaxBuy  = (float*)malloc(sizeof(float)*CTCandles);
	float* Delta3MinSell = (float*)malloc(sizeof(float)*CTCandles);
	float* Delta3MaxSell = (float*)malloc(sizeof(float)*CTCandles);
	for(int i = 0; i < CTCandles; i++)
	{
		Delta3MinBuy[i]  = 1000000;
		Delta3MaxBuy[i]  = -1000000;
		Delta3MinSell[i] = 1000000;
		Delta3MaxSell[i] = -1000000;
	}

	// Вычислим СМА и экстремумы
	/*// Экстремумы: минимумы и максимумы всегда чередуются и нет подряд */
	float* sma = (float*)malloc(sizeof(float)*100000);  // 100000 чтобы не выделять очень много памяти, например, при минутном таймфрейме
	ZeroMemory(sma, sizeof(float)*100000);

	int max_ct_extr = CurCountCandles[TM]/5;
	int ct_extr = 0;
	float* extrems = (float*)malloc(sizeof(float)*max_ct_extr);
	int*   indexes = (int*)malloc(sizeof(int)*max_ct_extr);
	ZeroMemory(extrems, sizeof(float)*max_ct_extr);
	ZeroMemory(indexes, sizeof(int)*max_ct_extr);

	int ct = PeriodSMA/2 + 1;
	int ind = 0;
	for(int i = PeriodSMA/2 + 1; i < CurCountCandles[TM] - PeriodSMA/2 - 1; i++)
	{
		float avg = 0;
		try{
		for(int p = i - PeriodSMA/2; p < i - PeriodSMA/2 + PeriodSMA; p++)
		{
			avg += candles[p].priceclose;
		}
		}
		catch(...)
		{int aa = 11;}
		avg /= (float)PeriodSMA;
		sma[ct] = avg;

		ct++;
		if(ct >= 100000 || i == CurCountCandles[TM] - PeriodSMA/2 - 2)
		{
            // Экстремумы
			for(int p = 1; p < ct && p < 100000 - 1; p++)
			{
				if(sma[p] == 0)
				   continue;

				if(ind + p > CTCandles)
				if(sma[p] > sma[p - 1] && sma[p] > sma[p + 1])
				{
					extrems[ct_extr] = sma[p];
					indexes[ct_extr] = ind + p;
					if(ct_extr < 100000)
					{
						try{
						FMain->points_numcndls[ct_extr] = ind + p;
						FMain->points_colors[ct_extr] = clTeal;
						FMain->count_points = ct_extr;
						}
						catch(...)
						{int aa = 11;}
                    }
					ct_extr++;
                }
            }

            ind += ct;
			ct = 0;
        }
    }

    // Вычислим ограничения
	for(int i = 0; i < ct_extr - 1; i++)
	{
		if(OUEDelta3->Checked)
		{
			float pr1 = candles[indexes[i]].priceclose;
			float pr2 = candles[indexes[i + 1]].priceclose;

			for(int p = 0; p < CTCandles; p++)
			{
				int cndl_ind = indexes[i] - 1 - p;
				if(cndl_ind < 3) continue;
				   //cndl_ind = 3;

				float delta3 = candles[cndl_ind].priceclose - candles[cndl_ind].priceopen +
							   candles[cndl_ind - 1].priceclose - candles[cndl_ind - 1].priceopen +
							   candles[cndl_ind - 2].priceclose - candles[cndl_ind - 2].priceopen;

				// Покупка
				if(pr2 - pr1 >= MinTP)
				{
					if(delta3 < Delta3MinBuy[p]) Delta3MinBuy[p] = delta3;
					if(delta3 > Delta3MaxBuy[p]) Delta3MaxBuy[p] = delta3;
				}
                // Продажа
				if(pr1 - pr2 >= MinTP)
				{
					if(delta3 < Delta3MinSell[p]) Delta3MinSell[p] = delta3;
					if(delta3 > Delta3MaxSell[p]) Delta3MaxSell[p] = delta3;
				}
            }
		}
	}

	//Создадим ограничивающие условия по всем данным по экстремумам
	AnsiString uslov_buy  = "";
	AnsiString uslov_sell = "";

	AnsiString tm = IntToStr(TM);
	for(int i = 0; i < CTCandles; i++)
	{
		AnsiString indx = IntToStr(0 - i);
		if(OUEDelta3->Checked)
		{
			try{
			uslov_buy  += "delta[" + tm + "; 3][" + indx + "] >= " + FloatToStr(Delta3MinBuy[i]) + " & ";
			uslov_buy  += "delta[" + tm + "; 3][" + indx + "] <= " + FloatToStr(Delta3MaxBuy[i]) + ", \r\n";

			uslov_sell += "delta[" + tm + "; 3][" + indx + "] >= " + FloatToStr(Delta3MinSell[i]) + " & ";
			uslov_sell += "delta[" + tm + "; 3][" + indx + "] <= " + FloatToStr(Delta3MaxSell[i]) + ", \r\n";
			}
			catch(...)
			{int aa = 11;}
        }
	}

	ResUsl->Text     = uslov_buy;
	ResUslSell->Text = uslov_sell;             FMain->count_points;
}
//---------------------------------------------------------------------------


//r0ket l0dable Dei Mudda
//10. August 2011
//für ELCH (DECT: 3524)

#include "basic/basic.h"
#include "usetable.h"

#define SP_LEN 90

void ram(void)
{
	char sp[SP_LEN];
	int button;
	int j=0;
	int x;
	int y;
	uint16_t rand=0;
	while(1){
		lcdClear();
		lcdPrintln("Dei Mudda ...");
		strcpy(&sp[0], "                                                                                          ");

		rand=getRandom();
		rand=rand%29;
		switch(rand) {
		  	case 0  :strcpy(&sp[0], "ist wie ein Senfglas, jeder darf mal seine wurst reinstecken");break;
          	  	case 1  :strcpy(&sp[0], "hat Beine wie ein Reh, zwar nicht so duenn aber genauso behaart");break; 
           	  	case 2  :strcpy(&sp[0], "arbeitet bei McDonalds als Fett");break;
           	  	case 3  :strcpy(&sp[0], "is so dumm, die schuettet Wasser uebern Computer damit sie im Internet Surfen kann");break;
            	  	case 4  :strcpy(&sp[0], "arbeitet beim Zirkus als Affe");break;
            	  	case 5  :strcpy(&sp[0], "steht vor der Rolltreppe und zaehlt die Stufen");break;
            	  	case 6  :strcpy(&sp[0], "traegt Gardinen als Reizwaesche");break;
            	  	case 7  :strcpy(&sp[0], "schminkt sich mit Eddings");break;
            	  	case 8  :strcpy(&sp[0], "kackt auf den Teppich, steckt Salzstangen rein und erklaert:Der Igel wohnt jetzt hier");break;
            	  	case 9  :strcpy(&sp[0], "Weisst du, wer zusammen ist? Die Augenbrauen von deiner Mudder");break;
            	  	case 10 :strcpy(&sp[0], "ist so haesslich, man band ihr damals ein Steak um den Hals damit wenigstens die Hunde mit ihr spielten.");break;
            	  	case 11 :strcpy(&sp[0], "ist so fett, wenn sie sich auf eine Waage stellt steht Bitte einer nach dem anderen drauf");break;
            	  	case 12 :strcpy(&sp[0], "ist so fett, sie benutzt eine Matratze als Tampon");break;
            	  	case 13 :strcpy(&sp[0], "ist so dumm, sie ist in einem Supermarkt eingesperrt worden und verhungerte");break;
            	  	case 14 :strcpy(&sp[0], "ist so haesslich, wenn sie strippt bekommt sie Geld damit sie sich wieder anzieht");break;
            	  	case 15 :strcpy(&sp[0], "disst einen Stein und der Stein gewinnt");break;
            	  	case 16 :strcpy(&sp[0], "sammelt haessliche Kinder");break;
            	  	case 17 :strcpy(&sp[0], "ist wie eine Tastatur Jeder hat sie mal angefast");break;
            	  	case 18 :strcpy(&sp[0], "gewinnt im Schwanzvergleich gegen deinen Vater");break;
            	  	case 19 :strcpy(&sp[0], "liegt bei Aldi unter der Kasse und macht PIEP");break;
            	  	case 20 :strcpy(&sp[0], "heisst Thorsten und ist die staerkste im Knast");break;
            	  	case 21 :strcpy(&sp[0], "programmiert in C#");break;
            	  	case 22 :strcpy(&sp[0], "verkauft im Wald Stoecke");break;
            	  	case 23 :strcpy(&sp[0], "ist so fett, andere Muetter kreisen um sie");break;
            	  	case 24 :strcpy(&sp[0], "sucht im Datenmuell nach Pfandflaschen");break;
            	  	case 25 :strcpy(&sp[0], "reitet auf ner Mig und ruft Jiiepie yajeee Schweinebacke");break;
            	  	case 26 :strcpy(&sp[0], "ist der Endgegner in Tekken 4");break;
            	  	case 27 :strcpy(&sp[0], "zerreisst Telefonbuecher bei Wetten dass");break;
            	  	case 28 :strcpy(&sp[0], "stolpert uebers Wlankabel");break;
			default :strcpy(&sp[0], "is so fett wenn sie in die luft springt bleibt sie stecken");break;
          	}

		x=0;
		y=8;
		for(j=0;j<SP_LEN;j++){
			if(sp[j]!=0){
				x=DoChar(x,y,sp[j]);
				if(x>90) {
					x=0;y+=8;
				}
			}
		}

		lcdRefresh();
		delayms(23);
		while((button=getInputRaw())==BTN_NONE)
			delayms(23);
		if(button==BTN_LEFT) return;
	}
}


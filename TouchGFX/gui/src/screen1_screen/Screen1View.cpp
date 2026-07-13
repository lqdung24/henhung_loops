#include <gui/screen1_screen/Screen1View.hpp>

Screen1View::Screen1View()
{

}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

// Trong file Screen1View.cpp
void Screen1View::updateCardId(const char* cardId)
{
	touchgfx::Unicode::snprintf(CardIDBuffer,
								CARDID_SIZE,
								"%02X%02X%02X%02X%02X",
								cardId[0], cardId[1], cardId[2], cardId[3], cardId[4]);


    CardID.invalidate();
}

void Screen1View::updateName(const char* name)
{
//	char buf[12] = "dung le";
	touchgfx::Unicode::fromUTF8(
	    (const uint8_t*)name,
	    FullNameTextBuffer,
	    FULLNAMETEXT_SIZE);

    FullNameText.invalidate();
}

void Screen1View::updateTime(const char* time)
{
	touchgfx::Unicode::fromUTF8(
	    (const uint8_t*) time,
		TimeBuffer,
	    TIME_SIZE);

	Time.invalidate();
}


void Screen1View::show_high_alcohol_dialog(int state)
{
	backgound_btn.setVisible((bool) state);
	alcohol_lv_high_text1.setVisible((bool) state);
	alcohol_lv_high_text2.setVisible((bool) state);

	backgound_btn.invalidate();
	alcohol_lv_high_text1.invalidate();
	alcohol_lv_high_text2.invalidate();
}

void Screen1View::show_blow_analog(int state, const char* time)
{
	backgound_btn.setVisible((bool) state);
	blow_sec.setVisible((bool) state);
	Blow_text.setVisible((bool) state);

	if(state){
		touchgfx::Unicode::fromUTF8(
		    (const uint8_t*) time,
			blow_secBuffer,
			BLOW_SEC_SIZE);
	}
}

void Screen1View::show_scan_card_dialog(int state)
{
	backgound_btn.setVisible((bool) state);
	scan_card.setVisible((bool) state);

	backgound_btn.invalidate();
	scan_card.invalidate();
}

void Screen1View::show_results(const char *raw_voltage, const char *alcohol_lv, const char *fine)
{
	touchgfx::Unicode::fromUTF8(
	    (const uint8_t*) alcohol_lv,
		ConvertedAlcoholTextBuffer,
		CONVERTEDALCOHOLTEXT_SIZE);

	touchgfx::Unicode::fromUTF8(
	    (const uint8_t*) raw_voltage,
		RawAlcoholTextBuffer,
		RAWALCOHOLTEXT_SIZE);

	touchgfx::Unicode::fromUTF8(
	    (const uint8_t*) fine,
		FineBuffer,
		FINE_SIZE);

	RawAlcoholText.invalidate();
	ConvertedAlcoholText.invalidate();
	Fine.invalidate();
}






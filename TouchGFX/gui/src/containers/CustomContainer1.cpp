#include <gui/containers/CustomContainer1.hpp>

CustomContainer1::CustomContainer1()
{

}

void CustomContainer1::initialize()
{
    CustomContainer1Base::initialize();
}

void CustomContainer1::setData(const Log_t& log)
{
    Unicode::fromUTF8((uint8_t*)log.id,
    		cardidBuffer,
			CARDID_SIZE);

    Unicode::fromUTF8((uint8_t*)log.name,
    		fullnameBuffer,
			FULLNAME_SIZE);

    Unicode::fromUTF8((uint8_t*)log.time,
    		timeBuffer,
			TIME_SIZE);

    Unicode::fromUTF8((uint8_t*)log.alcohol,
    		alcolhollvBuffer,
			ALCOLHOLLV_SIZE);

    invalidate();
}

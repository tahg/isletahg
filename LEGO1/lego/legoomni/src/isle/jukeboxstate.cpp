#include "jukeboxstate.h"

DECOMP_SIZE_ASSERT(JukeBoxState, 0x10)

// FUNCTION: LEGO1 0x1000f300
MxBool JukeBoxState::IsSerializable()
{
	return FALSE;
}

/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "platform.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#if defined(USE_RX_BIND)

#include "rx/crsf.h"
#include "rx_bind_phrase.h"
#include "rx/rx_spi_common.h"
#include "rx/srxl2.h"

static bool doRxBindPhrase(const char* bindPhrase, bool doBindPhrase)
{
    if (!doBindPhrase || bindPhrase == NULL || strlen(bindPhrase) == 0) {
        return false;
    }

    switch (rxRuntimeState.rxProvider) {
    default:
        return false;
    case RX_PROVIDER_SERIAL:
        switch (rxRuntimeState.serialrxProvider) {
        default:
            return false;
#if defined(USE_SERIALRX_CRSF)
        case SERIALRX_CRSF:
            crsfRxBindPhrase(bindPhrase);
            return true;
#endif
        }
        break;
    }
    return false;
}

bool startRxBindPhrase(const char* bindPhrase)
{
    return doRxBindPhrase(bindPhrase, true);
}

bool getRxBindPhraseSupported(void)
{
    return doRxBindPhrase(NULL, false);
}

#endif // USE_RX_BIND
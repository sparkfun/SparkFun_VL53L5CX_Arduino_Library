/*
  This is a library written for the ST VL53L5CX Time-of-flight sensor
  SparkFun sells these at its website:
  https://www.sparkfun.com/products/18642

  Do you like this library? Help support open source hardware. Buy a board!

  Written by Ricardo Ramos  @ SparkFun Electronics, October 22nd, 2021
  This file implements the VL53L5CX I2C driver class.
  
  This library uses ST's VL53L5CX driver and parts of Simon Levy's VL53L5CX
  Arduino library available at https://github.com/simondlevy/VL53L5/tree/main/src/st

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "SparkFun_VL53L5CX_IO.h"
#include "SparkFun_VL53L5CX_Library_Constants.h"

bool SparkFun_VL53L5CX_IO::begin(byte address, TwoWire &wirePort)
{
    _address = address;
    _i2cPort = &wirePort;
    return isConnected();
}

bool SparkFun_VL53L5CX_IO::isConnected()
{
    _i2cPort->beginTransmission(_address);
    if (_i2cPort->endTransmission() != 0)
        return (false);
    return (true);
}

uint8_t SparkFun_VL53L5CX_IO::writeMultipleBytes(uint16_t registerAddress, uint8_t *buffer, uint32_t packetLength)
{
    // This function was partially based on Simon Levy's VL53L5 library available at
    // https://github.com/simondlevy/VL53L5

    _i2cPort->beginTransmission(_address);
    _i2cPort->write(highByte(registerAddress));
    _i2cPort->write(lowByte(registerAddress));

    uint16_t updatedAddress = 0;

    for (uint32_t i = 0; i < packetLength; i++)
    {
        if ((i > 0) && (i < packetLength - 1) && (i % wireMaxPacketSize == 0))
        {
            _i2cPort->endTransmission(false);
            updatedAddress = registerAddress + i;
            _i2cPort->beginTransmission(_address);
            _i2cPort->write(highByte(updatedAddress));
            _i2cPort->write(lowByte(updatedAddress));
        }

        _i2cPort->write(buffer[i]);
    }
    return _i2cPort->endTransmission(true);
}

uint8_t SparkFun_VL53L5CX_IO::readMultipleBytes(uint16_t registerAddress, uint8_t *buffer, uint32_t packetLength)
{
    // This function was based on Simon Levy's VL53L5 library available at
    // https://github.com/simondlevy/VL53L5

    uint8_t status = 0;

    // Loop until the port is transmitted correctly
    do
    {
        _i2cPort->beginTransmission(_address);
        _i2cPort->write(highByte(registerAddress));
        _i2cPort->write(lowByte(registerAddress));

        status = _i2cPort->endTransmission(false);

        // Fix for some STM32 boards
        // Reinitialize the i2c bus with the default parameters
        // #ifdef ARDUINO_ARCH_STM32
        // if (status)
        // {
        //     _i2cPort->end();
        //     _i2cPort->begin();
        // }
        // #endif
        // End of fix

    } while (status != 0);

    uint32_t i = 0;
    if (packetLength > 32)
    {
        while (i < packetLength)
        {
            // If still more than wireMaxPacketSize bytes to go, wireMaxPacketSize, else the remaining number of bytes
            uint8_t current_read_size = (packetLength - i > wireMaxPacketSize ? wireMaxPacketSize : packetLength - i);
            _i2cPort->requestFrom(_address, current_read_size);
            while (_i2cPort->available())
            {
                buffer[i] = _i2cPort->read();
                i++;
            }
        }
    }
    else
    {
        _i2cPort->requestFrom(_address, packetLength);
        while (_i2cPort->available())
        {
            buffer[i] = _i2cPort->read();
            i++;
        }
    }

    return i != packetLength;
}

uint8_t SparkFun_VL53L5CX_IO::readSingleByte(uint16_t registerAddress)
{
    byte result;
    _i2cPort->beginTransmission(_address);
    _i2cPort->write(highByte(registerAddress));
    _i2cPort->write(lowByte(registerAddress));
    _i2cPort->endTransmission();
    _i2cPort->requestFrom(_address, 1U);
    result = _i2cPort->read();
    return result;
}

uint8_t SparkFun_VL53L5CX_IO::writeSingleByte(uint16_t registerAddress, uint8_t const value)
{
    _i2cPort->beginTransmission(_address);
    _i2cPort->write(highByte(registerAddress));
    _i2cPort->write(lowByte(registerAddress));
    _i2cPort->write(value);
    return _i2cPort->endTransmission();
}

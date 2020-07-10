// RHGenericSPI.h
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2011 Mike McCauley
// Contributed by Joanna Rutkowska
// $Id: RHGenericSPI.h,v 1.9 2020/01/05 07:02:23 mikem Exp $

#ifndef RHGenericSPI_h
#define RHGenericSPI_h

#include <RadioHead.h>

/////////////////////////////////////////////////////////////////////
/// \class RHGenericSPI RHGenericSPI.h <RHGenericSPI.h>
/// \brief Base class for SPI interfaces
///
/// This generic abstract class is used to encapsulate hardware or software SPI interfaces for 
/// a variety of platforms.
/// The intention is so that driver classes can be configured to use hardware or software SPI
/// without changing the main code.
///
/// You must provide a subclass of this class to driver constructors that require SPI.
/// A concrete subclass that encapsualates the standard Arduino hardware SPI and a bit-banged
/// software implementation is included.
///
/// Do not directly use this class: it must be subclassed and the following abstract functions at least 
/// must be implmented:
/// - begin()
/// - end() 
/// - transfer()
class RHGenericSPI 
{
public:

    /// \brief Defines constants for different SPI modes
    ///
    /// Defines constants for different SPI modes
    /// that can be passed to the constructor or setMode()
    /// We need to define these in a device and platform independent way, because the
    /// SPI implementation is different on each platform.
    typedef enum
    {
	DataMode0 = 0, ///< SPI Mode 0: CPOL = 0, CPHA = 0
	DataMode1,     ///< SPI Mode 1: CPOL = 0, CPHA = 1
	DataMode2,     ///< SPI Mode 2: CPOL = 1, CPHA = 0
	DataMode3,     ///< SPI Mode 3: CPOL = 1, CPHA = 1
    } DataMode;

    /// \brief Defines constants for different SPI bus frequencies
    ///
    /// Defines constants for different SPI bus frequencies
    /// that can be passed to setFrequency().
    /// The frequency you get may not be exactly the one according to the name.
    /// We need to define these in a device and platform independent way, because the
    /// SPI implementation is different on each platform.
    typedef enum
    {
	Frequency1MHz = 0,  ///< SPI bus frequency close to 1MHz
	Frequency2MHz,      ///< SPI bus frequency close to 2MHz
	Frequency4MHz,      ///< SPI bus frequency close to 4MHz
	Frequency8MHz,      ///< SPI bus frequency close to 8MHz
	Frequency16MHz      ///< SPI bus frequency close to 16MHz
    } Frequency;

    /// \brief Defines constants for different SPI endianness
    ///
    /// Defines constants for different SPI endianness
    /// that can be passed to setBitOrder()
    /// We need to define these in a device and platform independent way, because the
    /// SPI implementation is different on each platform.
    typedef enum
    {
	BitOrderMSBFirst = 0,  ///< SPI MSB first
	BitOrderLSBFirst,      ///< SPI LSB first
    } BitOrder;


    RHGenericSPI(Frequency frequency = Frequency1MHz, BitOrder bitOrder = BitOrderMSBFirst, DataMode dataMode = DataMode0)
        :
        _frequency(frequency),
        _bitOrder(bitOrder),
        _dataMode(dataMode)
    {
    }

    void setBitOrder(BitOrder bitOrder)
    {
        _bitOrder = bitOrder;
    }

    void setDataMode(DataMode dataMode)
    {
        _dataMode = dataMode; 
    }

    void setFrequency(Frequency frequency)
    {
        _frequency = frequency;
    }

protected:
    
    /// The configure SPI Bus frequency, one of Frequency
    Frequency    _frequency; // Bus frequency, one of Frequency

    /// Bit order, one of BitOrder
    BitOrder     _bitOrder;  

    /// SPI bus mode, one of DataMode
    DataMode     _dataMode;  
};
#endif

#ifndef __UBX_MESSAGE_PARSER_H__
#define __UBX_MESSAGE_PARSER_H__


class UbxMessageParser
{
public:
    UbxMessageParser()
    {
        Attach(NULL, 0);
    }
    
    ~UbxMessageParser()
    {
        Detach();
    }
    
    void Attach(uint8_t *buf, uint8_t bufLen)
    {
        buf_    = buf;
        bufLen_ = bufLen;
    }
    
    void Detach()
    {
        buf_    = NULL;
        bufLen_ = 0;
    }
    
    void Reset()
    {
        
    }
    
    uint8_t AddByte()
    {
        uint8_t retVal = 0;
        
        if (buf_)
        {
            retVal = 1;
            
            AddByteInternal(uint8_t b);
        }
        
        return retVal;
    }
    
    uint8_t GetMessage()
    {
        
    }

private:

    void AddByteInternal(uint8_t b)
    {
        if (state == State::LOOKING_FOR_HEADER)
        {
            //Log(F("LOOKING_FOR_HEADER"));
            
            // store this byte
            buf_[idx] = b;
            ++idx;

            // check if we received a byte previously
            // need 2 to match header
            if (idx == 1)
            {
                // nope, leave that byte stored and carry on
            }
            else
            {
                // yup, check if this is a valid header now
                if (buf_[0] == 0xB5 && buf_[1] == 0x62)
                {
                    // yup, move to next state
                    state = State::LOOKING_FOR_CLASS;
                }
                else
                {
                    //LogNNL(F("    discarding ")); Log(buf_[0], HEX);
                    
                    // nope, maybe the last byte that came in is the start,
                    // shift it to the start and carry on
                    buf_[0] = buf_[1];
                    --idx;
                }
            }
        }
        else if (state == State::LOOKING_FOR_CLASS)
        {
            //Log(F("LOOKING_FOR_CLASS"));

            buf_[idx] = b;
            ++idx;
            
            msgClass = b;

            //LogNNL(F("    class: "));  Log(b, HEX);

            state = State::LOOKING_FOR_ID;
        }
        else if (state == State::LOOKING_FOR_ID)
        {
            //Log("LOOKING_FOR_ID");

            buf_[idx] = b;
            ++idx;
            
            msgId = b;

            //LogNNL(F("    id: "));  Log(b, HEX);

            state = State::LOOKING_FOR_LEN;
        }
        else if (state == State::LOOKING_FOR_LEN)
        {
            //Log(F("LOOKING_FOR_LEN"));
            
            buf_[idx] = b;
            ++idx;

            if (idx == 6)
            {
                // we have the full size
                // we want to go from little endian wire format to host endian
                // start by making a network-byte-order 16-bit int, aka big endian
                uint16_t lenBigEndian;
                char *p = (char *)&lenBigEndian;
                p[0] = buf_[5];
                p[1] = buf_[4];

                len = PAL.ntohs(lenBigEndian);

                //LogNNL(F("    lenBigEndian: "));  Log(lenBigEndian);
                //LogNNL(F("    len         : "));  Log(len);

                // length does not include the header, class, id, length, or checksum fields.
                if (idx + len + 2 <= UBX_IN_BUF_SIZE)
                {
                    idxStopAt = idx + len + 2;

                    state = State::LOOKING_FOR_CHECKSUM;
                }
                else
                {
                    // Can't fit
                    cont = 0;

                    //Log(F("Message too large"));
                }
            }
            else
            {
                // Nothing to do, keep collecting
            }
        }
        else if (state == State::LOOKING_FOR_CHECKSUM)
        {
            //Log(F("LOOKING_FOR_CHECKSUM"));
            
            buf_[idx] = b;
            ++idx;

            if (idx == idxStopAt)
            {
                // Time to calculate and compare the checksum

                uint8_t idxChecksumStart = idxStopAt - 2;

                uint8_t ckA = 0;
                uint8_t ckB = 0;

                // did store the header (2 bytes total)
                // did store the class and id (2 bytes total)
                // did store the size (2 bytes total)
                // supposed to checksum from class and id forward, 
                
                for (uint8_t i = 2; i < idxChecksumStart; ++i)
                {
                    uint8_t b = buf_[i];
                    
                    ckA += b;
                    ckB += ckA;
                }

                // extract the message checksum
                uint8_t msgCkA = buf_[idxChecksumStart + 0];
                uint8_t msgCkB = buf_[idxChecksumStart + 1];

                //LogNNL("ckA, ckB: ");       LogNNL(ckA);    LogNNL(" "); LogNNL(ckB);    Log();
                //LogNNL("msgCkA, msgCkB: "); LogNNL(msgCkA); LogNNL(" "); LogNNL(msgCkB); Log();

                //StreamBlob(Serial, buf_, UBX_IN_BUF_SIZE, 1, 1);
                bufLen = idxStopAt;

                if (ckA == msgCkA && ckB == msgCkB)
                {
                    // success
                    found = 1;
                }
                else
                {
                    //Log(F("Checksum failed"));
                    msg.failReason = "Checksum failed";
                }

                cont = 0;
            }
            else
            {
                // Nothing to do, just pile on bytes until reaching the checksum
            }
        }

        
        
    }



    uint8_t *buf_;
    uint8_t  bufLen_;
    
    
    // Parsing state
    enum class State : uint8_t
    {
        LOOKING_FOR_HEADER = 0,
        LOOKING_FOR_CLASS,
        LOOKING_FOR_ID,
        LOOKING_FOR_LEN,
        LOOKING_FOR_CHECKSUM,
    };

    State state = State::LOOKING_FOR_HEADER;
    
    uint8_t bufIdx;

};


#endif  // __UBX_MESSAGE_PARSER_H__






























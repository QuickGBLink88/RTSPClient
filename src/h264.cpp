/*
对H264、H265的RTP包组帧，提取NALU单元
该文件的函数拷贝于FFmpeg源码文件：rtpdec_h264.c, rtpdec_hevc.c
*/

#include "stdafx.h"
#include "h264.h"

const bool using_donl_field = 0;

#define COUNT_NAL_TYPE(data, nal) do { } while (0)

static const uint8_t start_sequence[] = { 0, 0, 0, 1 };

//#define EINVAL          22
//#define ENOMEM          12
#define AV_RB16(x)  ((((const uint8_t*)(x))[0] << 8) | ((const uint8_t*)(x))[1])

#define AV_RB32(x)  ((((const uint8_t*)(x))[0] << 24) | \
 (((const uint8_t*)(x))[1] << 16) | \
 (((const uint8_t*)(x))[2] <<  8) | \
 ((const uint8_t*)(x))[3])


#define AV_WB32(p, d) do { \
 ((uint8_t*)(p))[3] = (d); \
 ((uint8_t*)(p))[2] = (d)>>8; \
 ((uint8_t*)(p))[1] = (d)>>16; \
 ((uint8_t*)(p))[0] = (d)>>24; } while(0)


int h264_handle_packet(const uint8_t *buf, int len, int & nIDR, uint8_t * outbuf, int & outlen)
{
	BOOL  bFrameEnd;
    uint8_t nal;
    uint8_t type;
    int result = 1;
	outlen = 0;

    if (!len) {
        TRACE("Empty H264 RTP packet\n");
        return 0;
    }
    nal  = buf[0];
    type = nal & 0x1f;

    assert(buf);

    /* Simplify the case (these are all the nal types used internally by
     * the h264 codec). */
    if (type >= 1 && type <= 23)
        type = 1;
    switch (type) {
    case 0:                    // undefined, but pass them through
    case 1:
		outlen = len + sizeof(start_sequence);
        
        memcpy(outbuf, start_sequence, sizeof(start_sequence));
        memcpy(outbuf + sizeof(start_sequence), buf, len);

		nIDR = (nal & 0x1f);
       // COUNT_NAL_TYPE(data, nal);
		bFrameEnd = 1;
        break;

    case 24:                   // STAP-A (one packet, multiple nals)
        // consume the STAP-A NAL
        buf++;
        len--;
        // first we are going to figure out the total size
        {
            int pass         = 0;
            int total_length = 0;
            uint8_t *dst     = NULL;

            for (pass = 0; pass < 2; pass++) {
                const uint8_t *src = buf;
                int src_len        = len;

                while (src_len > 2) {
                    uint16_t nal_size = AV_RB16(src);

                    // consume the length of the aggregate
                    src     += 2;
                    src_len -= 2;

                    if (nal_size <= src_len) {
                        if (pass == 0) {
                            // counting
                            total_length += sizeof(start_sequence) + nal_size;
                        } else {
                            // copying
                            assert(dst);
                            memcpy(dst, start_sequence, sizeof(start_sequence));
                            dst += sizeof(start_sequence);
                            memcpy(dst, src, nal_size);

                           // COUNT_NAL_TYPE(data, *src);
							nIDR = ((*src) & 0x1f);

                            dst += nal_size;
                        }
                    } else {
                        TRACE("nal size exceeds length: %d %d\n", nal_size, src_len);
						return 0;
                    }

                    // eat what we handled
                    src     += nal_size;
                    src_len -= nal_size;

                    if (src_len < 0)
					{
                        TRACE("Consumed more bytes than we got! (%d)\n", src_len);
						return 0;
					}
                }//while (src_len > 2)

                if (pass == 0) {
                    /* now we know the total size of the packet (with the
                     * start sequences added) */
					outlen = total_length;
                    dst = outbuf;
                } else {
                    assert(dst - outbuf == total_length);
                }
            }//for

			bFrameEnd = 1;
        }
        break;

    case 25:                   // STAP-B
    case 26:                   // MTAP-16
    case 27:                   // MTAP-24
    case 29:                   // FU-B
        TRACE("Unhandled type (%d) (See RFC for implementation details\n", type);
        result = 0;
        break;

    case 28:                   // FU-A (fragmented nal)
        buf++;
        len--;                 // skip the fu_indicator
        if (len > 1) {
            // these are the same as above, we just redo them here for clarity
            uint8_t fu_indicator      = nal;
            uint8_t fu_header         = *buf;
            uint8_t start_bit         = fu_header >> 7;
            uint8_t  end_bit = (fu_header & 0x40) >> 6;
            uint8_t nal_type          = fu_header & 0x1f;
            uint8_t reconstructed_nal;

            // Reconstruct this packet's true nal; only the data follows.
            /* The original nal forbidden bit and NRI are stored in this
             * packet's nal. */
            reconstructed_nal  = fu_indicator & 0xe0;
            reconstructed_nal |= nal_type;

            // skip the fu_header
            buf++;
            len--;

			nIDR = nal_type;

			if(end_bit)
				bFrameEnd = 1;

            if (start_bit){
                //COUNT_NAL_TYPE(data, nal_type);
				bFrameEnd = 0;
			}
            if (start_bit) {
                /* copy in the start sequence, and the reconstructed nal */
                outlen = sizeof(start_sequence) + sizeof(nal) + len;
                memcpy(outbuf, start_sequence, sizeof(start_sequence));
                outbuf[sizeof(start_sequence)] = reconstructed_nal;
                memcpy(outbuf + sizeof(start_sequence) + sizeof(nal), buf, len);
            } else {
				outlen = len;
                memcpy(outbuf, buf, len);
            }
        } else {
            TRACE( "Too short data for FU-A H264 RTP packet\n");
            result = 0;
        }
        break;

    case 30:                   // undefined
    case 31:                   // undefined
    default:
        TRACE("Undefined type (%d)\n", type);
        result = 0;
        break;
    }

    return result;
}


 int ff_h264_handle_aggregated_packet( int & nIDR, uint8_t * outbuf, int & outlen,
                                     const uint8_t *buf, int len,
                                     int skip_between, int *nal_counters,
                                     int nal_mask)
{
    int pass         = 0;
    int total_length = 0;
    uint8_t *dst     = NULL;
    int ret;

    // first we are going to figure out the total size
    for (pass = 0; pass < 2; pass++) {
        const uint8_t *src = buf;
        int src_len        = len;

        while (src_len > 2) {
            uint16_t nal_size = AV_RB16(src);

            // consume the length of the aggregate
            src     += 2;
            src_len -= 2;

            if (nal_size <= src_len) {
                if (pass == 0) {
                    // counting
                    total_length += sizeof(start_sequence) + nal_size;
                } else {
                    // copying
                    memcpy(dst, start_sequence, sizeof(start_sequence));
                    dst += sizeof(start_sequence);
                    memcpy(dst, src, nal_size);

					uint8_t nal_type = ((*src) & nal_mask);
					nIDR = nal_type; //PP
					
                    if (nal_counters)
                        nal_counters[(*src) & nal_mask]++;
                    dst += nal_size;
                }
            } else {
                TRACE("nal size exceeds length: %d %d\n", nal_size, src_len);
                return 0;
            }

            // eat what we handled
            src     += nal_size + skip_between;
            src_len -= nal_size + skip_between;
        }

        if (pass == 0) {
            /* now we know the total size of the packet (with the
             * start sequences added) */
			outlen = total_length;
            dst = outbuf;
        }
    }

    return outlen;
}

 int ff_h264_handle_frag_packet(int & nIDR, uint8_t * outbuf, int & outlen,
							   const uint8_t *buf, int len,
                               int start_bit, const uint8_t *nal_header,
                               int nal_header_len)
{
    int ret;
    int tot_len = len;
    int pos = 0;
    if (start_bit)
        tot_len += sizeof(start_sequence) + nal_header_len;

	outlen = tot_len;

	//nIDR = ((*nal_header) & nal_mask);

    if (start_bit) {
        memcpy(outbuf + pos, start_sequence, sizeof(start_sequence));
        pos += sizeof(start_sequence);
        memcpy(outbuf + pos, nal_header, nal_header_len);
        pos += nal_header_len;
    }
    memcpy(outbuf + pos, buf, len);
    return outlen;
}

 int h264_handle_packet_fu_a(int & nIDR, uint8_t * outbuf, int & outlen,
                                   const uint8_t *buf, int len,
                                   int *nal_counters, int nal_mask)
{
    uint8_t fu_indicator, fu_header, start_bit, nal_type, nal;

    if (len < 3) {
        TRACE("Too short data for FU-A H264 RTP packet\n");
        return 0;
    }

    fu_indicator = buf[0];
    fu_header    = buf[1];
    start_bit    = fu_header >> 7;
    nal_type     = fu_header & 0x1f;
    nal          = fu_indicator & 0xe0 | nal_type;

	//nIDR = (nal_type & nal_mask);

    // skip the fu_indicator and fu_header
    buf += 2;
    len -= 2;

    if (start_bit && nal_counters)
        nal_counters[nal_type & nal_mask]++;
    return ff_h264_handle_frag_packet(nIDR, outbuf, outlen, buf, len, start_bit, &nal, 1);
}



int hevc_handle_packet(const uint8_t *buf, int len, int & nIDR, uint8_t * outbuf, int & outlen)
{
    const uint8_t *rtp_pl = buf;
    int tid, lid, nal_type;
    int first_fragment, last_fragment, fu_type;
    uint8_t new_nal_header[2];
    int res = 1;
	outlen = 0;

    /* sanity check for size of input packet: 1 byte payload at least */
    if (len < RTP_HEVC_PAYLOAD_HEADER_SIZE + 1) {
        TRACE("Too short RTP/HEVC packet, got %d bytes\n", len);
        return 0;
    }

    /*
     * decode the HEVC payload header according to section 4 of draft version 6:
     *
     *    0                   1
     *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
     *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *   |F|   Type    |  LayerId  | TID |
     *   +-------------+-----------------+
     *
     *      Forbidden zero (F): 1 bit
     *      NAL unit type (Type): 6 bits
     *      NUH layer ID (LayerId): 6 bits
     *      NUH temporal ID plus 1 (TID): 3 bits
     */
    nal_type =  (buf[0] >> 1) & 0x3f;
    lid  = ((buf[0] << 5) & 0x20) | ((buf[1] >> 3) & 0x1f);
    tid  =   buf[1] & 0x07;

    /* sanity check for correct layer ID */
    if (lid) {
        /* future scalable or 3D video coding extensions */
        TRACE("Multi-layer HEVC coding\n");
       // return 0;
    }

    /* sanity check for correct temporal ID */
    if (!tid) {
        TRACE("Illegal temporal ID in RTP/HEVC packet\n");
       // return 0;
    }

    /* sanity check for correct NAL unit type */
    if (nal_type > 50) {
        TRACE("Unsupported (HEVC) NAL type (%d)\n", nal_type);
        return 0;
    }

    switch (nal_type) {
    /* video parameter set (VPS) */
    case 32:
    /* sequence parameter set (SPS) */
    case 33:
    /* picture parameter set (PPS) */
    case 34:
    /*  supplemental enhancement information (SEI) */
    case 39:
    /* single NAL unit packet */
    default:
        /* sanity check for size of input packet: 1 byte payload at least */
        if (len < 1) {
            TRACE("Too short RTP/HEVC packet, got %d bytes of NAL unit type %d\n",
                   len, nal_type);
            return 0;
        }


		outlen = sizeof(start_sequence) + len;
		
        /* A/V packet: copy start sequence */
        memcpy(outbuf, start_sequence, sizeof(start_sequence));
        /* A/V packet: copy NAL unit data */
        memcpy(outbuf + sizeof(start_sequence), buf, len);

		TRACE("single nal_type: %d \n", nal_type);
	
		nIDR = nal_type;
		

        break;
    /* aggregated packet (AP) - with two or more NAL units */
    case 48:
        /* pass the HEVC payload header */
        buf += RTP_HEVC_PAYLOAD_HEADER_SIZE;
        len -= RTP_HEVC_PAYLOAD_HEADER_SIZE;

        /* pass the HEVC DONL field */
        if (using_donl_field) {
            buf += RTP_HEVC_DONL_FIELD_SIZE;
            len -= RTP_HEVC_DONL_FIELD_SIZE;
        }

        res = ff_h264_handle_aggregated_packet(nIDR, outbuf, outlen, buf, len,
                                               using_donl_field ?
                                               RTP_HEVC_DOND_FIELD_SIZE : 0,
                                               NULL, 0x3f);
        if (res <= 0)
            return res;

        break;
    /* fragmentation unit (FU) */
    case 49:
        /* pass the HEVC payload header */
        buf += RTP_HEVC_PAYLOAD_HEADER_SIZE;
        len -= RTP_HEVC_PAYLOAD_HEADER_SIZE;

        /*
         *    decode the FU header
         *
         *     0 1 2 3 4 5 6 7
         *    +-+-+-+-+-+-+-+-+
         *    |S|E|  FuType   |
         *    +---------------+
         *
         *       Start fragment (S): 1 bit
         *       End fragment (E): 1 bit
         *       FuType: 6 bits
         */
        first_fragment = buf[0] & 0x80;
        last_fragment  = buf[0] & 0x40;
        fu_type        = buf[0] & 0x3f;

        /* pass the HEVC FU header */
        buf += RTP_HEVC_FU_HEADER_SIZE;
        len -= RTP_HEVC_FU_HEADER_SIZE;

        /* pass the HEVC DONL field */
        if (using_donl_field) {
            buf += RTP_HEVC_DONL_FIELD_SIZE;
            len -= RTP_HEVC_DONL_FIELD_SIZE;
        }

       // TRACE(" FU type %d with %d bytes\n", fu_type, len);

        /* sanity check for size of input packet: 1 byte payload at least */
        if (len <= 0) {
            if (len < 0) {
                TRACE("Too short RTP/HEVC packet, got %d bytes of NAL unit type %d\n",
                       len, nal_type);
                return 0;
            } else {
                return 0;
            }
        }

        if (first_fragment && last_fragment) {
            TRACE("Illegal combination of S and E bit in RTP/HEVC packet\n");
            return 0;
        }

		TRACE("fu_type = %d \n", fu_type);

        new_nal_header[0] = (rtp_pl[0] & 0x81) | (fu_type << 1);
        new_nal_header[1] = rtp_pl[1];

        res = ff_h264_handle_frag_packet(nIDR, outbuf, outlen, buf, len, first_fragment,
                                         new_nal_header, sizeof(new_nal_header));

		nIDR = fu_type;
        break;
    /* PACI packet */
    case 50:
        /* Temporal scalability control information (TSCI) */
        TRACE("PACI packets for RTP/HEVC\n");
        res = 0;
        break;
    }


    return res;
}
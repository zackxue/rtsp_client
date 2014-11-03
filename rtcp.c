#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "rtcp.h"
#include "utils.h"
#include "rtp.h"


static void ParseSenderDescribe(char *buf, uint32_t len, char *sdes);
static void ParseSenderReport(char *buf, uint32_t len, char *srh);
static void ParseSenderReport(char *buf, uint32_t len, char *srh)
{
    SenderReport *sr = (SenderReport *)(srh);
    char *ptr = buf;

    sr->ntp_timestamp_msw = GET_32(ptr);
    ptr += 4;
    sr->ntp_timestamp_lsw = GET_32(ptr);
    ptr += 4;
    sr->rtp_timestamp = GET_32(ptr);
    ptr += 4;
    sr->senders_packet_count = GET_32(ptr);
    ptr += 4;
    sr->senders_octet_count = GET_32(ptr);
    ptr += 4;

    if ((ptr-buf) > len){
        fprintf(stderr, "%s: length error!\n", __func__);
        return;
    }
#ifdef RTSP_DEBUG
    if (1) {
        printf("Timestamp MSW   : 0x%x (%u)\n", sr->ntp_timestamp_msw, sr->ntp_timestamp_msw);
        printf("Timestamp LSW   : 0x%x (%u)\n", sr->ntp_timestamp_lsw, sr->ntp_timestamp_lsw);
        printf("RTP timestamp  : 0x%x (%u)\n", sr->rtp_timestamp, sr->rtp_timestamp);
        printf("Sender Packet Count : %u\n", sr->senders_packet_count);
        printf("Sender Octet Count : %u\n", sr->senders_octet_count);
    }
#endif

    return;
}

static void ParseSenderDescribe(char *buf, uint32_t len, char *sdes)
{
#if 0
    SDES *sd = (SDES *)(sdes);
    char *ptr = buf;


    sd->identifier = GET_32(ptr);
    ptr += 4;

#ifdef RTSP_DEBUG
    if (1){
        printf("     ID       : %u\n", sd->identifier);
        /*printf("     Type     : %i\n", sd->start);*/
        /*printf("     Length   : %i\n", sd->length);*/
        /*printf("     Type 2   : %i\n", sd->end);*/
    }
#endif
#endif

    return;
}

uint32_t ParseRtcp(char *buf, uint32_t len)
{
    uint32_t i = 0;
    char *ptr = buf;
    while (i < len) {
        RtcpHeader *rtcph = (RtcpHeader *)(ptr);
#if 0
        start = i;
        /* RTCP */
        rtcph[idx].version   = (buf[i] >> 6)&0x03;
        rtcph[idx].padding   = ((buf[i] & 0x20) >> 5)&0x01;
        rtcph[idx].rc        = buf[i] & 0x1F;
        i++;
        rtcph[idx].type      = (buf[i]);
        /* length */
        uint32_t length = GET_16(&buf[i]);
        PUT_16(&rtcph[idx].length[0], length);
        i += 2;
#endif
#ifdef RTSP_DEBUG
        if (1){
            printf("RTCP Version  : %i\n", rtcph->version);
            printf("     Padding  : %i\n", rtcph->padbit);
            printf("     RC: %i\n", rtcph->rc);
            printf("     Type     : %i\n", rtcph->type);
            printf("     Length     : %i\n", GET_16(rtcph->length));
        }
#endif
        ptr += sizeof(RtcpHeader);
        uint32_t length = GET_16(rtcph->length)*4;
        if (rtcph->type == RTCP_SR){
            RtcpSR rsr;
            rsr.ssrc = GET_32(ptr);
            ptr += 4;
            length -= 4;
            ParseSenderReport(ptr, length, (char *)&(rsr.sr));
        }else if (rtcph->type == RTCP_SDES){/* source definition */
            SDES sdes;
            ParseSenderDescribe(ptr, length, (char *)&sdes);
        }else if (rtcph->type == RTCP_BYE){
            return RTCP_BYE;
        }
        ptr += length;
        i += length + sizeof(RtcpHeader);
    }

    return RTCP_SR;
}


static void InitRtcpHeader(RtcpHeader *ch, uint32_t type, uint32_t rc, uint32_t bytes_len)
{
    RtcpHeaderSetVersion(ch,2);
    RtcpHeaderSetPadbit(ch,0);
    RtcpHeaderSetPacketType(ch, type);
    RtcpHeaderSetRc(ch,rc);	/* as we don't yet support multi source receiving */
    RtcpHeaderSetLength(ch,(bytes_len/4)-1);

	return;
}

static uint32_t InitRtcpReceiveReport(char *buf, uint32_t size){

	RtcpRR *rrr=(RtcpRR *)buf;
	if (size < sizeof(RtcpRR)) return 0;
	InitRtcpHeader(&rrr->hdr, RTCP_RR, 1, sizeof(RtcpRR));
    rrr->ssrc = htonl(RTCP_SSRC);
	return sizeof(RtcpRR);
}

uint32_t RtcpReceiveReport(char *buf, uint32_t len, RtpSession *sess)
{
    RtpStats *rtpst = (RtpStats *)(&sess->stats);

    /* init Receive Report */
    InitRtcpReceiveReport(buf, len);

	RtcpRR *rrr=(RtcpRR *)buf;
    /*
     * Calcs for expected and lost packets
     */
    uint32_t extended_max;
    uint32_t expected;
    extended_max = rtpst->rtp_received + rtpst->highest_seq;
    expected = extended_max - rtpst->first_seq + 1;
    rtpst->rtp_cum_lost = expected - rtpst->rtp_received - 1;
#if 0
    /* Fraction */
    uint32_t expected_interval;
    uint32_t received_interval;
    uint32_t lost_interval;
    uint8_t fraction;

    expected_interval = expected - rtpst->rtp_expected_prior;
    rtpst->rtp_expected_prior = expected;

    received_interval = rtpst->rtp_received - rtpst->rtp_received_prior;
    rtpst->rtp_received_prior = rtpst->rtp_received;
    lost_interval = expected_interval - received_interval;
    if (expected_interval == 0 || lost_interval <= 0){
        fraction = 0;
    }else{
        fraction = (lost_interval << 8) / expected_interval;
    }
#endif
    ReceiveReport *rr = &rrr->rr;
    rr->ssrc = sess->ssrc;
    rr->fl_cnpl = 0x00;

    /* RTCP: SSRC Contents: Extended highest sequence */
    PUT_16(rr->cycle, 0x01);
    PUT_16(rr->highestseq, rtpst->highest_seq);

    /* RTCP: SSRC Contents: interarrival jitter */
    rr->interarrival_jitter = rtpst->jitter;

    /* RTCP: SSRC Contents: Last SR timestamp */
    rr->lsr = rtpst->last_rcv_SR_ts;

    /* RTCP: SSRC Contents: Timestamp delay */

    struct timeval now;
    double delay;
    if (rtpst->last_rcv_SR_ts != 0) {
        gettimeofday(&now,NULL);
        delay= (now.tv_sec - rtpst->last_rcv_SR_time.tv_sec) +
               ((now.tv_usec - rtpst->last_rcv_SR_time.tv_usec)*1e-6);
        delay= (delay * 65536);
        rtpst->delay_snc_last_SR = (uint32_t) delay;
    }
    rr->delay_snc_last_sr = rtpst->delay_snc_last_SR;

    return sizeof(RtcpRR);
}
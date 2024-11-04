/*
Name: Asya Akkus 
Case Network ID: aya29
File Name: proj4.c
Date Created: 10/28/2024
Description: file implements the r, i, s, t, and m options 
that process a packet trace file in different ways. 
*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>
#include <netinet/ip.h>



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/ethernet.h>

#define ARG_R  0x1  // Root directory (trace file)
#define ARG_I  0x2  // Trace information mode
#define ARG_S  0x4  // Size analysis mode
#define ARG_T  0x8  // TCP packet printing mode
#define ARG_M  0x10 // Traffic matrix mode

char *trace_file = NULL; 
unsigned short cmd_line_flags = 0;

#define MAX_PKT_SIZE        1600

/* meta information, using same layout as trace file */
struct meta_info
{
    unsigned int secs;
    unsigned int usecs;
    unsigned short caplen;
    unsigned short ignored;
};

/* record of information about the current packet */
struct pkt_info
{
    unsigned short caplen;      /* from meta info */
    double now;                 /* from meta info */
    unsigned char pkt [MAX_PKT_SIZE];
    struct ether_header *ethh;  /* ptr to ethernet header, if present,
                                   otherwise NULL */
    struct iphdr *iph;          /* ptr to IP header, if present, 
                                   otherwise NULL */
    struct tcphdr *tcph;        /* ptr to TCP header, if present,
                                   otherwise NULL */
    struct udphdr *udph;        /* ptr to UDP header, if present,
                                   otherwise NULL */
};

void usage(char *progname) {
    fprintf(stderr, "%s -r trace_file [-i | -s | -t | -m]\n", progname);
    fprintf(stderr, "   -r    Packet trace file (must be present)\n");
    fprintf(stderr, "   -i    Trace information mode\n");
    fprintf(stderr, "   -s    Size analysis mode\n");
    fprintf(stderr, "   -t    TCP packet printing mode\n");
    fprintf(stderr, "   -m    Traffic matrix mode\n");
    exit(1);
}

void validateargs(int argc, char *argv[]) {
    // The trace file must always be present
    if ((cmd_line_flags & ARG_R) == 0 || trace_file == NULL) {
        fprintf(stderr, "error: -r option must be present\n");
        usage(argv[0]);
    }

    // Only one mode must be selected (-i, -s, -t, -m)
    int mode_count = !!(cmd_line_flags & ARG_I) + !!(cmd_line_flags & ARG_S) +
                     !!(cmd_line_flags & ARG_T) + !!(cmd_line_flags & ARG_M);
                     
    if (mode_count == 0) {
        fprintf(stderr, "error: one mode option (-i, -s, -t, -m) must be selected\n");
        usage(argv[0]);
    }
    if (mode_count > 1) {
        fprintf(stderr, "error: multiple mode options cannot be selected simultaneously\n");
        usage(argv[0]);
    }
}

void parseargs(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "r:istm")) != -1) {
        switch (opt) {
            case 'r':
                cmd_line_flags |= ARG_R;
                trace_file = optarg;
                if (trace_file == NULL) {
                    fprintf(stderr, "error: -r option must be followed by a packet trace file.\n");
                    exit(1);
                }
                break;
            case 'i':
                cmd_line_flags |= ARG_I;
                break;
            case 's':
                cmd_line_flags |= ARG_S;
                break;
            case 't':
                cmd_line_flags |= ARG_T;
                break;
            case 'm':
                cmd_line_flags |= ARG_M;
                break;
            case '?':
                usage(argv[0]);
                exit(1);
            default:
                usage(argv[0]);
                exit(1);
        }
    }

    validateargs(argc, argv);
}


unsigned short next_packet (int fd, struct pkt_info *pinfo)
{
    struct meta_info meta;
    int bytes_read;

    memset (pinfo,0x0,sizeof (struct pkt_info));
    memset (&meta,0x0,sizeof (struct meta_info));

    /* read the meta information */
    bytes_read = read (fd,&meta,sizeof (meta));
    if (bytes_read == 0)
        return (0);
    if (bytes_read < sizeof (meta))
        fprintf (stderr, "cannot read meta information");
    pinfo->caplen = ntohs (meta.caplen);
    /* TODO: set pinfo->now based on meta.secs & meta.usecs */
    if (pinfo->caplen == 0)
        return (1);
    if (pinfo->caplen > MAX_PKT_SIZE)
        fprintf(stderr, "packet too big");
    /* read the packet contents */
    bytes_read = read (fd,pinfo->pkt,pinfo->caplen);
    if (bytes_read < 0)
        fprintf(stderr, "error reading packet");
    if (bytes_read < pinfo->caplen)
        fprintf(stderr, "unexpected end of file encountered");
    if (bytes_read < sizeof (struct ether_header))
        return (1);
    pinfo->ethh = (struct ether_header *)pinfo->pkt;
    pinfo->ethh->ether_type = ntohs (pinfo->ethh->ether_type);
    if (pinfo->ethh->ether_type != ETHERTYPE_IP)
        /* nothing more to do with non-IP packets */
        return (1);
    if (pinfo->caplen == sizeof (struct ether_header))
        /* we don't have anything beyond the ethernet header to process */
        return (1);
    /* TODO:
       set pinfo->iph to start of IP header
       if TCP packet, 
          set pinfo->tcph to the start of the TCP header
          setup values in pinfo->tcph, as needed
       if UDP packet, 
          set pinfo->udph to the start of the UDP header,
          setup values in pinfo->udph, as needed */
    pinfo->iph = (struct iphdr *)(pinfo->pkt + sizeof(struct ether_header));
    
    return (1);
}

/*
Trace Information Mode
When your program is invoked with the “-i” option, it will operate in “trace information” mode. This mode
provides a high-level summary of the trace file. After processing each packet in the trace, your program will
print several pieces of information to standard output in this format:
[trace_filename] [first_time] [trace_duration] [total_pkts] [IP_pkts]
The value of “first time” is the timestamp (found in meta information for each packet in the trace) of the
first packet in the trace file. This will be printed in seconds-since-epoch format and include 6 decimal places
of precision—e.g., “1103112609.135350”. The value of “trace duration” is the duration of the trace file—i.e.,
the timestamp of the last packet in the file minus first time. The duration should be printed in seconds and
to 6 decimal places of precision. The value of “total pkts” is the number of packets found in the packet
trace file, reported as a decimal number with no padding. Finally, the value of “ip pkts” is the number of
IPv4 packets found in the packet trace file, reported as a decimal number with no padding. Note: ip pkts ≤
total pkts. Also, note that the braces in the above format are to denote variables and should not appear in
your output.
The labels will be presented in lower-case, as shown. A single space will separate all fields / words on each
line. Further, no additional white space at the beginning or end of the lines may be printed.
Hint: We determine whether a packet is an IPv4 packet or not by consulting the “type” field in the Ethernet
header. See packet trace format document for more information.
Sample output:
./proj4 -r trace1.dmp -i
trace1.dmp 1103112609.132870 0.002484 4 3
*/

void i_option() {
    FILE *f = fopen(trace_file, "rb"); 
    
    if (f == NULL) {
        fprintf(stderr, "error opening the file. Please try a different file"); 
        exit(1); 
    }

    struct pkt_info pinfo; 

    int fd = fileno(f); 
    int total_pkts = 0;
    int ip_pkts = 0; 
    double first_time = 0.0; 
    double last_time = 0.0; 

    while (next_packet(fd, &pinfo)) {
        if (total_pkts == 0) {
            first_time = pinfo.now;
        }

        last_time = pinfo.now;  

        total_pkts++;
        
        // Check if the packet is an IP packet
        if (pinfo.ethh->ether_type == ETHERTYPE_IP) {
            ip_pkts++;
        }
    }
    
    double trace_duration = last_time - first_time;

    // Print the results in the specified format
    printf("%s %.6f %.6f %d %d\n", trace_file, first_time, trace_duration, total_pkts, ip_pkts);

    fclose(f);
}

/*
When your program is invoked with the “-s” option, it will operate in “size” mode. In this mode, you will
print length information about each IPv4 packet in the packet trace file. That is, if the Ethernet type field
indicates a packet is not an IPv4 packet, you must ignore it. Likewise, if the Ethernet header is not present
in the packet trace you will ignore the packet. Each packet will yield a single line of output in this format:
ts caplen ip_len iphl transport trans_hl payload_len
The fields are defined as follows:
• ts: This field is the timestamp of the packet, which is included with the packet’s meta information
(see packet trace format document). Print this as a decimal number to 6 decimal places of precision.
• caplen: This is the number of bytes from the original packet that have been “captured” in the packet
trace. This value is included with the packet’s meta information. This value must be printed as an
unpadded decimal number.
• ip len: This is the total length (in bytes) of the IPv4 packet (from the total length field in the IPv4
header). This should be printed as an unpadded decimal number.
If the IPv4 header is not included in the packet trace file, you must print a singe dash (“-”) for this
field.
• iphl: This is the total length (in bytes) of the IPv4 header—which can be determined from the IPv4
header.
As with the ip len field, if the IPv4 header is not present in the packet trace, this field will be printed
as a “-”.
• transport: This indicates the transport protocol in use for this packet. A field in the IPv4 header
will indicate which transport is in use. This field should be printed as a “U” for UDP packets and a
“T” for TCP packets. For all other protocols, this value will be a question mark (“?”).
This value will be a “-” if the IPv4 header is not included in the packet trace.
• trans hl: This is the total number of bytes occupied by the TCP or UDP header written as an
unpadded decimal value.
For other transport protocols your program must print a single question mark (“?”) for this field.
When the IPv4 header is not included in the trace the size of the transport’s header cannot be deter-
mined and therefore this field will be a single dash (“-”).
When the TCP or UDP header is not included in the packet trace the entry in this field will be a single
dash (“-”).
• payload len: The final value to be printed for each packet is the number of application layer payload
bytes present in the original packet. This will be determined by starting with the ip len value and
subtracting all IPv4 and transport layer header bytes.
Again, if the IPv4 header is not present, this value cannot be determined and a “-” will be printed.
Likewise, when the packet is a protocol other than TCP or UDP, report “?” for this field.
Finally, when the TCP or UDP header is not available in the trace file, print a “-” in this field.
Each IPv4 packet must produce a single line of output. The fields must be separated by a single space and
no additional whitespace may appear at the beginning or end of the line.
Sample output:
./proj4 -s -r trace2.dmp
1103112609.132870 54 240 20 T 20 200
1103112610.465345 34 1000 20 T - -
1103112615.436115 43 308 24 U 8 276
1103112618.029221 20 - - - - -
*/
void s_option() {
    FILE *f = fopen(trace_file, "rb"); 

    if (f == NULL) {
        fprintf(stderr, "error opening the file. Please try a different file"); 
        exit(1); 
    }

    struct pkt_info pinfo; 
    int fd = fileno(f); 
    int ip_pkts = 0; 
   
    while (next_packet(fd, &pinfo)) {  

        // Check if the packet is an IP packet
        if (pinfo.ethh->ether_type == ETHERTYPE_IP) {
            int ip_len = ntohs(pinfo.iph->tot_len);

            printf("%i %i", pinfo.caplen, ip_len); 
            ip_pkts++; 
        }
        else {
            ip_pkts++; 
        }
    }

}

int main(int argc, char *argv[]) {
    parseargs(argc, argv);

    if (cmd_line_flags == ARG_R+ARG_I) {
        i_option(); 
    }

    if (cmd_line_flags == ARG_R+ARG_S) {
        s_option(); 
    }
    return 0; 
}

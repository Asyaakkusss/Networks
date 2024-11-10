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
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>     


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
char transport[2] = "-"; 
char transhl_questionmk[2] = "?"; 
char transhl_dash[2] = "-"; 
char syn_status[2] = "N"; 
#define MAX_PKT_SIZE 1600
#define FOUR 4
#define NEG_ONE -1
#define SIX 6
#define SEVENTEEN 17
#define EIGHT 8
//i option variables 
int total_pkts = 0;
int ip_pkts = 0; 
double first_time = 0.0; 
double last_time = 0.0; 

/*info for the traffic info for m option
members: source IP, destination IP, total packets, traffic volume*/
struct traffic_info 
{
    struct in_addr src_ip;    
    struct in_addr dst_ip;    
    unsigned int total_pkts;  
    unsigned int traffic_volume; 
}; 

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
    pinfo->now = (double)ntohl(meta.secs) + (double)ntohl(meta.usecs)/(double)1e6; 
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
    pinfo->iph = (struct iphdr *)(pinfo->pkt + sizeof(struct ether_header));

    if (pinfo->iph->protocol == IPPROTO_TCP) {
        pinfo->tcph = (struct tcphdr *)(pinfo->pkt + sizeof(struct ether_header) + (pinfo->iph->ihl * 4)); 
    }

    if (pinfo->iph->protocol == IPPROTO_UDP) {
        pinfo->udph = (struct udphdr *)(pinfo->pkt + + sizeof(struct ether_header) + (pinfo->iph->ihl * 4)); 
    }
    
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

tcp-1.trace 1431329876.000005 0.000000 1 1
*/

void i_option(FILE *f) {

    struct pkt_info pinfo; 

    int fd = fileno(f); 

    while (next_packet(fd, &pinfo)) {
        if (total_pkts == 0) {
            first_time = pinfo.now;
        }

        last_time = pinfo.now;  

        total_pkts++;
        
        // Check if the packet is an IP packet
        if (pinfo.ethh == NULL) {
            continue; 
        }
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
ts: This field is the timestamp of the packet, which is included with the packet’s meta information
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
Sample output:
./proj4 -s -r trace2.dmp
1103112609.132870 54 240 20 T 20 200
1103112610.465345 34 1000 20 T - -
1103112615.436115 43 308 24 U 8 276
1103112618.029221 20 - - - - -
1431329876.000005 54 40 20 T 20 0
*/
void s_option(FILE *f) {

    struct pkt_info pinfo; 
    int fd = fileno(f); 
   
    while (next_packet(fd, &pinfo)) {  

        // Check if the packet is an IP packet
        if (pinfo.ethh == NULL) {
            continue; 
        }

        // Check if the packet is an IP packet
        else if (pinfo.ethh->ether_type == ETHERTYPE_IP) {
            
            double ts = pinfo.now; //ts 
            int ip_len = (pinfo.iph != NULL) ? ntohs(pinfo.iph->tot_len) : NEG_ONE; //ip_len
            int iphl = (pinfo.iph != NULL) ? (pinfo.iph->ihl) * FOUR: NEG_ONE; //iphl
            int caplen = pinfo.caplen; //caplen
            int trans_hl; 
            int payload; 

            if (pinfo.iph == NULL) {
                strcpy(transport, "-"); //transport doesn't exist bc ip header doesn't exist 
            }

            else if (pinfo.iph->protocol == SIX) {
                strcpy(transport, "T"); //transport (tcp)
            }

            else if (pinfo.iph->protocol == SEVENTEEN) {
                strcpy(transport, "U"); //transport (udp)
            }

            else {
                strcpy(transport, "?"); //transport (neither udp nor tcp)
            }


            if (pinfo.iph != NULL && pinfo.iph->protocol == SIX) {
                trans_hl = (pinfo.tcph->doff) * FOUR; //trans hl for tcp (this is wrong)
                payload = ip_len - (iphl + trans_hl); 

            }

            if (pinfo.iph != NULL && pinfo.iph->protocol == SEVENTEEN) {
                trans_hl = EIGHT; //trans hl for udp (this is wrong)
                payload = ip_len - (iphl + trans_hl);
            }

            /*calculating payload length*/

            printf("%.6f %i ", ts, caplen); 
            if (ip_len <= 0) {
                printf("%s ", "-"); 
            }
            else {
                printf("%i ", ip_len);
            }
            if (iphl <= 0) {
                printf("%s ", "-"); 
            }
            else {
                printf("%i ", iphl);
            }

            printf("%s ", transport); 
            if (pinfo.iph != NULL && (pinfo.iph->protocol == SIX || pinfo.iph->protocol == SEVENTEEN)) {
                printf("%d %i\n", trans_hl, payload); 
            }
            else if (pinfo.iph != NULL) {
                printf("%s %s\n", transhl_questionmk, transhl_questionmk); 
            }

            else {
                printf("%s %s\n", transhl_dash, transhl_dash); 
            }

        }
        
    }

}

/*
When your program is run with the “-t” option it will operate in “packet printing mode”. In this mode, you
will output a single line of information about each TCP packet in the packet trace file. Non-TCP packets
will be ignored. Further, TCP packets that do not have the TCP header in the packet trace file will be
ignored. The output format for this mode is:
ts src_ip src_port dst_ip dst_port ip_ttl ip_id syn window seqno
The fields are defined as follows:
• ts: This field is the timestamp of the packet, which is included with the packet’s meta information
(see packet trace format document). Print this as a decimal number to 6 decimal places of precision.
• src ip: This is the dotted-quad version of the source IPv4 address. E.g., “192.168.1.43”. Print the
numbers in decimal and do not pad.
• src port: This is the unpadded decimal version of TCP’s source port number.
• dst ip: This is the dotted-quad version of the destination IPv4 address. E.g., “192.168.1.43”. Print
the numbers in decimal and do not pad.
• dst port: This is the unpadded decimal version of TCP’s destination port number.
• ip ttl: This is the unpadded decimal value in IP’s TTL field.
• ip id: This is the unpadded decimal value in IP’s ID field.
• syn: This must indicate whether the TCP packet’s SYN bit is set or not. When the bit is turned on
(i.e., is “1”) this field in the output will be “Y”. When the bit is turned off (i.e., is “0”) this field in
the output will be “N”.
• window: This is the unpadded decimal value in TCP’s advertised window field. (Disregard any window
scaling that may be happening.)
• seqno: This is the unpadded decimal value in TCP’s sequence number field.
Each TCP packet must produce a single line of output. The fields must be separated by a single space and
no additional whitespace may appear at the beginning or end of the line.
Sample output:
./proj4 -t -r some-example.dmp
1103112609.132870 192.168.1.2 4512 192.168.100.34 80 127 4912 Y 16384 3828024032
1103112610.983425 192.168.100.34 80 192.168.1.2 4512 63 11783 N 32961 37709858
*/
void t_option(FILE *f) {

    struct pkt_info pinfo; 
    int fd = fileno(f); 
    int ip_pkts; 
    double ts; 
    char src_ip[INET_ADDRSTRLEN];  
    int src_port; 
    char dst_ip[INET_ADDRSTRLEN]; 
    int dst_port; 
    int ip_ttl; 
    int ip_id; 
    int window; 
    uint32_t seqno; 

    while (next_packet(fd, &pinfo)) {
        if (pinfo.tcph == NULL) {
            continue; 
        }

        else {
        ts = pinfo.now; //ts
        struct in_addr s_addr;
        struct in_addr d_addr; 
        s_addr.s_addr = pinfo.iph->saddr;
        d_addr.s_addr = pinfo.iph->daddr; 
        inet_ntop(AF_INET, &pinfo.iph->saddr, src_ip, INET_ADDRSTRLEN); //src ip 
        src_port = ntohs(pinfo.tcph->source); //src port 
        inet_ntop(AF_INET, &pinfo.iph->daddr, dst_ip, INET_ADDRSTRLEN); //dst ip
        dst_port = ntohs(pinfo.tcph->dest); //dst port 
        ip_ttl = pinfo.iph->ttl; //ttl 
        ip_id = ntohs(pinfo.iph->id);  
        
        if(pinfo.tcph->th_flags & TH_SYN) {
            strcpy(syn_status, "Y"); 
        }

        else {
            strcpy(syn_status, "N"); 
        }

        window = ntohs(pinfo.tcph->window); 
        seqno = ntohl(pinfo.tcph->seq);  
        
        }

        printf("%.6f %s %i %s %i %i %i %s %i %u\n", ts, src_ip, src_port, dst_ip, dst_port, ip_ttl, ip_id, syn_status, window, seqno); 

    }

}


void update_traffic_struct(struct traffic_info **table, int *count, struct in_addr src_ip, struct in_addr dst_ip, int pkt_size) {
    // Check if an entry for the source IP and destination IP already exists
    for (int i = 0; i < *count; i++) {
        if ((*table)[i].src_ip.s_addr == src_ip.s_addr && (*table)[i].dst_ip.s_addr == dst_ip.s_addr) {
            (*table)[i].total_pkts += 1;          
            (*table)[i].traffic_volume += pkt_size; 
            return;
        }
    }

    // If it doesn't exist, add it in
    *table = realloc(*table, (*count + sizeof(unsigned char)) * sizeof(struct traffic_info));
    if (*table == NULL) {
        perror("Failed to realloc memory for traffic table");
        exit(EXIT_FAILURE);
    }

    (*table)[*count].src_ip = src_ip;
    (*table)[*count].dst_ip = dst_ip;
    (*table)[*count].total_pkts = sizeof(unsigned char);
    (*table)[*count].traffic_volume = pkt_size;
    (*count)++;
}

void process_trace_file(FILE *f, struct traffic_info **table, int *count) {
    struct pkt_info pinfo;

    while (next_packet(fileno(f), &pinfo)) {
        if (pinfo.tcph == NULL) {
            continue; 
        }

        // Calculate application layer data size
        int ip_header_len = pinfo.iph->ihl * FOUR;
        int tcp_header_len = pinfo.tcph->doff * FOUR;
        int pkt_size = ntohs(pinfo.iph->tot_len) - (ip_header_len + tcp_header_len);

        struct in_addr src_ip, dst_ip;
        src_ip.s_addr = pinfo.iph->saddr;
        dst_ip.s_addr = pinfo.iph->daddr;

        update_traffic_struct(table, count, src_ip, dst_ip, pkt_size);
    }
}

void print_traffic_info(struct traffic_info *table, int count) {
    char src_ip_mod[INET_ADDRSTRLEN]; 
    char dst_ip_mod[INET_ADDRSTRLEN]; 
    for (int i = 0; i < count; i++) {
        inet_ntop(AF_INET, &(table[i].src_ip), src_ip_mod, INET_ADDRSTRLEN); 
        inet_ntop(AF_INET, &(table[i].dst_ip), dst_ip_mod, INET_ADDRSTRLEN); 

        printf("%s %s %d %d\n",
            src_ip_mod, 
            dst_ip_mod,
            table[i].total_pkts,
            table[i].traffic_volume);
    }
}

int main(int argc, char *argv[]) {
    parseargs(argc, argv);

    FILE *f = fopen(trace_file, "rb"); 
    
    if (f == NULL) {
        fprintf(stderr, "error opening the file. Please try a different file\n"); 
        exit(1); 
    }

    if (cmd_line_flags == ARG_R + ARG_I) {
        i_option(f); 
    }

    if (cmd_line_flags == ARG_R + ARG_S) {
        s_option(f); 
    }

    if (cmd_line_flags == ARG_R + ARG_T) {
        t_option(f); 
    }

    if (cmd_line_flags == ARG_R + ARG_M) {

        struct traffic_info *table = NULL; 
        int count = 0; 

        process_trace_file(f, &table, &count); 
        print_traffic_info(table, count); 

        free(table); 
        fclose(f); 
    }
    return 0; 
}
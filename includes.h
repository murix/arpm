struct pkt_struct {
        unsigned char eth_dst[6];
        unsigned char eth_src[6];
        unsigned short eth_proto;
        unsigned short int arp_hw_type;
        unsigned short int arp_proto;
        unsigned char arp_hw_len;
        unsigned char arp_proto_len; 
        unsigned short arp_oper;
        unsigned char arp_hw_src[6];
        unsigned char arp_ip_src[4];
        unsigned char arp_hw_dst[6];
        unsigned char arp_ip_dst[4];
};

struct iplist_struct {
        unsigned long int ip;
        struct iplist_struct * next;
};

struct list_struct {
        unsigned char hw[ETH_ALEN];
        struct iplist_struct * iplist;
        struct iplist_struct * lastip;
        struct list_struct * next;
} * head = NULL, * tail = NULL;

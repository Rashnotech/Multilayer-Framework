#!/usr/bin/env python3
"""a module that implement packet inspection and signature matching"""

from scapy.all import sniff


attack_signatures = [
        {'src_port': 80, 'dst_port': 8080, 'protocol': 'TCP'}
        ]



def detect_packet(packet):
    for signature in attack_signatures:
        if packet.sport == signature['src_port']\
        and packet.dport == signature['dst_port']\
        and packet.proto == signature['protocol']:
            return True
    return False



def start_sniffing():
    sniff(filter='tcp', prn=detect_packet, store=0)

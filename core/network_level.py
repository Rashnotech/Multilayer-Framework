#!/usr/bin/env python3
"""a module that handles packet inspection"""
from scapy.all import sniff
from typing import List
import logging


logging.basicConfig(filename='threat.log',
                    level=logging.INFO,
                    format="%(asctime)s - %(message)s")


class NetworkIntrusion:
    """
    A network class that handle real-time monitoring of network
    packet on a network
    Args:
        ipdaddress: List of machine address
        port: scan ports
    """

    def __init__(self, ip_address: List[str], port: List[int]) -> None:
        """initialization method"""
        self.ip_address = ip_address or []
        self.port = port or []
        self.signatures = [
                {'src_port': 80, 'dst_port': 8080, 'protocol': 'TCP'},
                {'src_port': 443, 'dst_port': 22, 'protocol': 'TCP'},
                {'src_port': 53, 'dst_port': 53, 'protocol': 'UDP'}
                ]

    def detect_packet(self, packet) -> bool:
        """
        Check if the incoming packet matches any known attack signature
        Args:
            packet: The packet captured by scapy's sniff method.
        Returns:
            bool: True if the packet matches an attack signature otherwise False

        """
        for signature in signatures:
            if packet.haslayer('IP'):
                if (packet.haslayer('TCP') and
                        packet.sport == signature['src_port']
                        and packet.dport == signature['dst_port']
                        and packet.proto == signature['protocol']):
                    logging.info('Suspicious TCP detected from {} to {}'
                            .format(packet[IP].src, packet[IP].dst))
                    return True
                elif (packet.haslayer('UDP') and
                        packet.sport == signature['src_port']
                        and packet.dport == signature['dst_port']):
                    logging.info('Suspicious UDP activity detected from {} to {}'
                            .format(packet[IP].src, packet[IP].dst))
                    return True
        return False



    def start_sniffing(self) -> None:
        """
        Start sniffing packets on the network and check them for intrusion.
        """
        sniff(filter='tcp or udp', prn=self.detect_packet, store=0)

    def connect_devices(self, subnet) -> List[str]:
        """
        Discover all devices on the network using ARP scanning
        """
        arp_request = ARP(pdst=subnet)
        broadcast = Ether(dst='ff:ff:ff:ff:ff:ff')
        arp_request_broadcast = broadcast / arp_request

        _list = srp(arp_request_broadcast, timeout=5, verbose=False)[0]
        devices = []

        for elem in _list:
            devices.append({'ip': elem[1].psrc, 'mac': elem[1].hwsrc})
        self.ip_address = [device['ip'] for device in devices]

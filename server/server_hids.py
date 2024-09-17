#!/usr/bin/env python3
""" a module that combine file integrity montoring and log monitoring"""
import thread
import time
from core.host_level import HostLevel


files_to_monitor = ['/etc/passwd', '/etc/ssh/sshd_config']

authorized_process = ['sshd', 'systemd', 'nginx', 'python3']
hostlevel = HostLevel()

def start_hids():
    """start monitoring file integrity in thread"""
    file_monitor_thread = threading.Thread(target=hostlevel.monitor_files,
                                           args=(files_to_monitor,))
    file_monitor_thread.daemon = True
    file_monitor_thread.start()

    process_monitor_thread = threading.Thread(target=hostlevelmonitor_processes,
                                              args=(authorized_process,))
    process_monitor_thread.daemon = True
    process_monitor_thread.start()

    hostlevel.monitor_syslog()

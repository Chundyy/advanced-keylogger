from cryptography.fernet import Fernet
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
import base64
import os
import logging

class LogDecryptor:
    def __init__(self, encryption_key: str):
        """Initialize with encryption key"""
        self.logger = logging.getLogger('decryptor')
        self.kdf = PBKDF2HMAC(
            algorithm=hashes.SHA256(),
            length=32,
            salt=b'secure_salt',  # Should be unique per deployment
            iterations=100000
        )
        self.cipher = self._init_cipher(encryption_key)

    def _init_cipher(self, key: str) -> Fernet:
        """Initialize Fernet cipher with key derivation"""
        try:
            # Key derivation for additional security
            derived_key = base64.urlsafe_b64encode(
                self.kdf.derive(key.encode())
            )
            return Fernet(derived_key)
        except Exception as e:
            self.logger.error(f"Key derivation failed: {str(e)}")
            raise

    def decrypt_log(self, encrypted_data: bytes) -> str:
        """
        Decrypt received logs with error handling
        Returns: Decrypted UTF-8 string
        """
        try:
            decrypted = self.cipher.decrypt(encrypted_data)
            return decrypted.decode('utf-8')
        except Exception as e:
            self.logger.error(f"Decryption failed: {str(e)}")
            raise ValueError("Invalid or corrupted encrypted data")

    @staticmethod
    def generate_key() -> str:
        """Generate new Fernet key for setup"""
        return Fernet.generate_key().decode()

from cryptography.fernet import Fernet

# Gerar nova chave (execute isto uma vez)
key = Fernet.generate_key()
print("Sua chave Fernet válida:", key.decode())




//"your-encryption-key-here-32-bytes-long"

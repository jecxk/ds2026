import os
from xmlrpc.server import SimpleXMLRPCServer
from xmlrpc.server import SimpleXMLRPCRequestHandler
from xmlrpc.client import Binary

BASE_DIR = os.path.abspath("server_storage")


class RequestHandler(SimpleXMLRPCRequestHandler):
    rpc_paths = ("/RPC2",)


def ensure_dir():
    if not os.path.exists(BASE_DIR):
        os.makedirs(BASE_DIR, exist_ok=True)


def list_files():
    ensure_dir()
    return os.listdir(BASE_DIR)


def upload_file(filename, data):
    ensure_dir()
    filepath = os.path.join(BASE_DIR, filename)
    with open(filepath, "wb") as f:
        f.write(data.data)
    print(f"[SERVER] Uploaded {filename}")
    return True


def download_file(filename):
    ensure_dir()
    filepath = os.path.join(BASE_DIR, filename)
    if not os.path.exists(filepath):
        raise FileNotFoundError("File does not exist.")
    with open(filepath, "rb") as f:
        return Binary(f.read())


def delete_file(filename):
    ensure_dir()
    filepath = os.path.join(BASE_DIR, filename)
    if os.path.exists(filepath):
        os.remove(filepath)
        print(f"[SERVER] Deleted {filename}")
        return True
    return False


def main():
    ensure_dir()
    server = SimpleXMLRPCServer(("0.0.0.0", 9000), requestHandler=RequestHandler, allow_none=True)
    print("[SERVER] RPC File Server running on port 9000")

    server.register_function(list_files, "list_files")
    server.register_function(upload_file, "upload_file")
    server.register_function(download_file, "download_file")
    server.register_function(delete_file, "delete_file")

    server.serve_forever()


if __name__ == "__main__":
    main()

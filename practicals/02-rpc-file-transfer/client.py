import sys
from xmlrpc.client import ServerProxy, Binary


def usage():
    print("Usage:")
    print("  python client.py list")
    print("  python client.py upload <local> <remote>")
    print("  python client.py download <remote> <local>")
    print("  python client.py delete <remote>")


server = ServerProxy("http://localhost:9000/RPC2", allow_none=True)


def main():
    if len(sys.argv) < 2:
        usage()
        return

    cmd = sys.argv[1]

    if cmd == "list":
        print(server.list_files())

    elif cmd == "upload" and len(sys.argv) == 4:
        with open(sys.argv[2], "rb") as f:
            data = f.read()
        server.upload_file(sys.argv[3], Binary(data))
        print("Uploaded.")

    elif cmd == "download" and len(sys.argv) == 4:
        data = server.download_file(sys.argv[2])
        with open(sys.argv[3], "wb") as f:
            f.write(data.data)
        print("Downloaded.")

    elif cmd == "delete" and len(sys.argv) == 3:
        server.delete_file(sys.argv[2])
        print("Deleted.")

    else:
        usage()


if __name__ == "__main__":
    main()

import os, requests, sys, shutil

#REMOTE_URL = 'http://mywebsitev2.test:81/update-server-list'
REMOTE_URL = 'https://svistunov.dev/update-server-list'
DATA_DIR = sys.path[0] + '/../Data/'

if len(sys.argv) < 2:
    print("You should pass password as argument.")
    sys.exit(1)
filename = sys.path[0] + '/favicons'
shutil.make_archive(filename, 'zip', DATA_DIR + 'Favicons')

with open(DATA_DIR + 'servers.xml','rb') as file_servers_xml, open(filename + '.zip','rb') as icons_zip:
    files = {
        'file': file_servers_xml,
        'icons': icons_zip,
    }
    values = {'password': sys.argv[1]}

    r = requests.post(REMOTE_URL, files=files, data=values,  headers={'Accept': 'application/json'})
    print(r.text)

os.remove(filename + '.zip')
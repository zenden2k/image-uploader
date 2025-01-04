import requests, sys, shutil

#REMOTE_URL = 'http://mywebsitev2.test:81/update-server-list'
REMOTE_URL = 'https://svistunov.dev/update-server-list'
DATA_DIR = sys.path[0] + '/../Data/'

if len(sys.argv) < 2:
    print("You should pass password as argument.")
    sys.exit(1)
filename = sys.path[0] + '/favicons'
shutil.make_archive(filename, 'zip', DATA_DIR + 'Favicons')

files = {
    'file': open(DATA_DIR + 'servers.xml','rb'),
    'icons': open(filename + '.zip','rb'),
}
values = {'password': sys.argv[1]}

r = requests.post(REMOTE_URL, files=files, data=values,  headers={'Accept': 'application/json'})
print(r.text)
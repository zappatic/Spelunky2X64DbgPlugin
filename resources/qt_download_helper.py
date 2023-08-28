#!/usr/bin/env python 
#-*- coding: utf-8 -*-

import urllib.request
import re
import html
import ssl

#main_download_url = 'https://osdn.net/projects/x64dbg/storage/qt/Qt5.6.3-msvc2013-installed.7z'
main_download_url = 'https://sourceforge.net/projects/x64dbg/files/qt/Qt5.6.3-msvc2013-installed.7z/download'

# response = urllib.request.urlopen(main_download_url)
# data = response.read().decode('utf-8')

# mirror_redirect_url = 'https://osdn.net'
# match = re.search(r'meta http-equiv="refresh" content="[0-9]+; url=(.*?)">', data)
# if match != None:
#     mirror_redirect_url = mirror_redirect_url + html.unescape(match.group(1))
# else:
#     raise Exception('Could not determine mirror url from OSDN page')

# ssl._create_default_https_context = ssl._create_unverified_context
urllib.request.urlretrieve(main_download_url, "C:\Qt\qt563.7z")

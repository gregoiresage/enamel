import os
import json
import collections
import re
from jinja2 import Environment
from jinja2 import FileSystemLoader

def cvarname(name):
    """Convert a string to a valid c variable name (remove space,commas,slashes/...)."""
    return re.sub(r'([^\w\s]| )', '_', name)

def getid(item):
    """Return an identifier for the given config item, takes 'id' if it exists or 'appKey'"""
    return item['id'] if 'id' in item else item['appKey']

def generate(appinfo='appinfo.json', configFile='src/js/config.json', outputDir='src/generated', outputFileName='enamel'):
    """Generates C helpers from a Clay configuration file"""
    # create output folder
    if not os.path.exists(outputDir):
        os.makedirs(outputDir)

    # crete jinja environment
    env = Environment(loader = FileSystemLoader([os.path.join(os.path.dirname(__file__), 'templates')]), trim_blocks=True, lstrip_blocks=True)

    # add custom filters
    env.filters['cvarname'] = cvarname
    env.filters['getid']    = getid

    # loads appinfo file
    appinfo_content=open(appinfo)
    appinfo_content=json.load(appinfo_content)

    # loads config file
    config_content=open(configFile)
    config_content=json.load(config_content)

    # render templates
    for template in ['enamel.h.jinja', 'enamel.c.jinja'] : 
    	extension = ".h" if template.endswith('h.jinja') else ".c" 
        f = open("%s/%s%s" % (outputDir, outputFileName, extension), 'w')
        f.write(env.get_template(template).render({'filename' : outputFileName, 'config' : config_content, 'appinfo' : appinfo_content}))
        f.close()

import argparse
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generates C helpers from a Clay configuration file')
    parser.add_argument('--appinfo', action='store', default='appinfo.json', help='Path to appinfo.json')
    parser.add_argument('--config', action='store', default='src/js/config.json', help='Path to Clay configuration file') 
    parser.add_argument('--folder', action='store', default='src/generated', help='Generation folder') 
    parser.add_argument('--filename', action='store', default='enamel', help='Name for the generated files without extension (default : enamel)') 
    result = parser.parse_args()
    generate(appinfo=result.appinfo, configFile=result.config, outputDir=result.folder, outputFileName=result.filename)
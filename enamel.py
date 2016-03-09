import os
import json
import collections
import re
import sys
import array

try:
    from jinja2 import Environment
    from jinja2 import FileSystemLoader
except ImportError as e:
    if 'pebble-sdk' in sys.prefix :
        message = 'Jinja2 module is missing, you probably forgot to patch your current sdk\n'
        message += 'Fix the problem by executing the following command and relaunch your build:\n\n'
        message += 'pip install --target=%s/lib/python2.7/site-packages/ -r %s/requirements.txt\n' % (sys.prefix, os.path.dirname(os.path.abspath(__file__)))
        print message
        sys.exit(-1)
    else :
        raise e

def cvarname(name):
    """Convert a string to a valid c variable name (remove space,commas,slashes/...)."""
    return re.sub(r'([^\w\s]| )', '_', name)

def getid(item):
    """Return an identifier for the given config item, takes 'id' if it exists or 'appKey'"""
    return item['id'] if 'id' in item else item['appKey']

def maxdictsize(item):
    """Return the maximum size of the item in the dictionary"""
    size = 0
    if item['type'] == 'select' or item['type'] == 'radiogroup' :
        for option in item['options'] :
            size = max(size, len(str(option['value'])) + 1)
    elif item['type'] == 'checkboxgroup' :
        for option in item['options'] :
            size += len(str(option['value'])) + 1
    return size

def defaulttobytearray(item):
    """Convert the array of default values to an array of bytes (only relevant for checkboxgroup)"""
    res = ''
    if item['type'] == 'checkboxgroup' :
        arr = []
        for value in item['defaultValue'] :
            arr += map(ord,value)
            arr += [0]
        res = '{'
        for a in arr :
            res += str(a) + ','
        res += '}'
    return res

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
    env.filters['maxdictsize']  = maxdictsize
    env.filters['defaulttobytearray'] = defaulttobytearray

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

def enamel(task):
    generate(configFile=task.inputs[0].abspath(), outputDir=task.generator.bld.bldnode.abspath(), outputFileName=os.path.splitext(os.path.basename(task.outputs[0].abspath()))[0])

import argparse
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generates C helpers from a Clay configuration file')
    parser.add_argument('--appinfo', action='store', default='appinfo.json', help='Path to appinfo.json')
    parser.add_argument('--config', action='store', default='src/js/config.json', help='Path to Clay configuration file') 
    parser.add_argument('--folder', action='store', default='.', help='Generation folder') 
    parser.add_argument('--filename', action='store', default='enamel', help='Name for the generated files without extension (default : enamel)') 
    result = parser.parse_args()
    generate(appinfo=result.appinfo, configFile=result.config, outputDir=result.folder, outputFileName=result.filename)

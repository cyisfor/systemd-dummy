import sys,os

template = open("template.mk").read()
name = sys.argv[1]
print(template.replace(

#!/usr/bin/env python
# -*- coding: utf-8 -*-

from setuptools import setup
from distutils.core import Extension

version = '0.1'
MOD = 'handiso'
source = 'handiso.c'

setup(name = MOD,
      version = version,
      description = "a library for efficiently mapping poker hands to and from a tight set of indices",
      classifiers=[
        "Programming Language :: Python",
      ],
      author = 'wrapper C from @Kevin Waugh',
      ext_modules = [Extension(MOD, sources = ['handiso.c', 'src/hand_index.c', 'src/deck.c',])],
      install_requires=[
          'setuptools',
      ]
)

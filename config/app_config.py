#!/usr/bin/env python3
"""app configuration"""
import os
from datetime import timedelta


class Config:
    """ a class for config """
    SECRET_KEY = "think about it"
    JWT_SECRET_KEY = "think about it"
    JWT_TOKEN_LOCATION = ['headers']
    JWT_ACCESS_TOKEN_EXPIRES = timedelta(days=2)

    @classmethod
    def init_app(cls, app):
        pass

#!/usr/bin/env python3
"""
a module that handle the list restricted IP
"""
from models.base import Base, BaseModel
from sqlalchemy import Column, String
from uuid import uuid4


class BlackList(Base, BaseModel):
    """
    A class that handle blacklist on database
    """

    ipaddress = Column(String(100), nullable=False)
    mode = Column(String(60), nullable=False)

    def __init__(self, **kwargs):
        """initialization method"""
        super().__init__(self, **kwargs)

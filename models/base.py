#!/usr/bin/env python3
"""base model"""
from datetime import datetime, timezone
from uuid import uuid4
from sqlalchemy import DateTime, Column, String, Integer
from sqlalchemy.ext.declarative import declarative_base


Base = declarative_base()


class BaseModel:
    """
    A base class model

    Args:
        kwargs: key word arguments
    """
    id = Column(String(100), unique=True, primary_key=True,
                default= lambda: str(uuid4), nullable=False)
    created_at = Column(DateTime, default=datetime.now(timezone.utc),
            onupdate=datetime.now(timezone.utc), nullable=False)
    updated_at = Column(DateTime, default=datetime.now(timezone.utc),
            nullable=False, onupdate=datetime.now(timezone.utc))

    def __init__(self, **kwargs) -> None:
        """initialization method"""
        if kwargs:
            for attr, val in kwargs.items():
                if attr in ['created_at', 'updated_at']:
                    setattr(self, attr, datetime.strptime(val,
                                        '%Y-%m-%dT%H:%M:%S.%f'))
                elif attr == '__class__':
                    del attr
                else:
                    setatrr(self, attr, val)
        else:
            self.id = str(uuid4())
            self.created_at = datetime.now(timezone.utc)
            self.updated_at = datetime.now(timezone.utc)

    
    def __str__(self) -> str:
        """return a representation of the base model"""
        return '[{}] ({}) {}'.format(self.__class__.__name__,
                                     self.id, self.__dict__)

    def to_dict(self) -> dict:
        """return dictionary containing all key/value"""
        new_dict = self.__dict__.copy()
        new_dict['__class__'] = self.__class__.__name__
        new_dict['created_at'] = self.created_at.isoformat()
        new_dict['updated_at'] = self.updated_at.isoformat()
        if '_sa_instance_state' in new_dict:
            del new_dict['_sa_instance_state']
        return new_dict


    def save(self) -> None:
        """update public instance attribute updated at"""
        from models import storage
        self.updated_at = datetime.now(timezone.utc)
        storage.new(self)
        storage.save()

    def delete(self):
        """delete current instance of the storage"""
        from models import storage
        storage.delete(self)

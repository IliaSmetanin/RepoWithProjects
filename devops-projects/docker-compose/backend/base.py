from sqlalchemy import Column, Integer, String, create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker

import os

# connection to db from ENV VARS
HOST = os.environ.get('DB_HOST', 'localhost')

# connection address
# user:         postgres
# password:     postgres (from .db.env)
# host:         db (from 'DB_HOST' or 'localhost')
# db name:      postgres
SQLALCHEMY_DATABASE_URL = f"postgresql://postgres:postgres@{HOST}/postgres"

# all connections go through engine
engine = create_engine(
    SQLALCHEMY_DATABASE_URL
)

# session fabric for creating temporary connections
SessionLocal = sessionmaker(
    autocommit=False, autoflush=False, bind=engine
)

Base = declarative_base()

# table declaration
class User(Base):
    __tablename__ = "users"

    id = Column(Integer, primary_key=True, index=True)
    login = Column(String, index=True)
    email = Column(String, unique=True, index=True)
    hashed_password = Column(String) # the only member that isn't generated automatically
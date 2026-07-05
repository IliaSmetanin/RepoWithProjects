# from base import Base, engine
from base import Base, engine, SessionLocal, User

# creating a table
Base.metadata.create_all(bind=engine)

db = SessionLocal()

users = [
    User(login='pudge', email='hook@gmail.com', hashed_password='meta'),
    User(login='morphling', email='adaptive@gmail.com', hashed_password='hard_carry')
]

db.add(users[0])
db.add(users[1])
db.commit()
db.close() # because of opening it in "db = SessionLocal()"
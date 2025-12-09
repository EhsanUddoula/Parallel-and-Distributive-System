import sqlite3
import os

def create_database():
    # Remove existing database
    if os.path.exists('jokes.db'):
        os.remove('jokes.db')
    
    # Connect to SQLite database (creates if not exists)
    conn = sqlite3.connect('jokes.db')
    cursor = conn.cursor()
    
    # Create jokes table
    cursor.execute('''
        CREATE TABLE jokes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            setup TEXT NOT NULL,
            punchline TEXT NOT NULL
        )
    ''')
    
    # Insert jokes
    jokes = [
        ("Turnip", "Turnip the heat. It's freezing."),
        ("Echo", "Echo who? Exactly!"),
        ("Boo", "Don't cry, it's just a joke!"),
        ("Lettuce", "Lettuce in, it's cold outside!"),
        ("Harry", "Harry up and answer the door!"),
        ("Annie", "Annie thing you can do, I can do better!"),
        ("Luke", "Luke through the keyhole and see!"),
        ("Dwayne", "Dwayne the bathtub, I'm drowning!"),
        ("Candice", "Candice door open or what?"),
        ("Atch", "Bless you!"),
        ("Cow says", "No, a cow says mooooo!"),
        ("Tank", "You're welcome!"),
        ("Hawaii", "I'm good, Hawaii you?"),
        ("Alaska", "Not much, Alaska later!"),
        ("Irish", "Irish you would stop with these jokes!"),
        ("Orange", "Orange you glad I didn't say banana?"),
        ("Banana", "Banana split so we can both have some!"),
        ("Who", "Is there an owl in here?"),
        ("Woo", "Don't get so excited, it's just a joke!")
    ]
    
    cursor.executemany('INSERT INTO jokes (setup, punchline) VALUES (?, ?)', jokes)
    
    # Commit and close
    conn.commit()
    conn.close()
    print(f"Created database with {len(jokes)} jokes")

if __name__ == "__main__":
    create_database()
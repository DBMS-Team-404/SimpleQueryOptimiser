SELECT name, amount FROM users LEFT JOIN orders ON users.id = orders.user_id;

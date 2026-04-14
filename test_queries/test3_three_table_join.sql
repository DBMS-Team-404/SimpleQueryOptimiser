SELECT name, role_name, amount
FROM users
JOIN orders ON users.id = orders.user_id
JOIN roles  ON users.role_id = roles.id;

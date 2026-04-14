SELECT *
FROM users AS u
JOIN orders AS o ON u.id = o.user_id
JOIN roles AS r ON u.role_id = r.id
JOIN products AS p ON o.id = p.id
WHERE r.role_name = 'admin';
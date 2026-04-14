SELECT u1.name, u2.name, oo.amount, p.name
FROM users AS u1
JOIN users AS u2 ON u1.age = u2.age
JOIN orders AS oo ON u1.id = oo.user_id
JOIN products AS p ON oo.order_date = p.id
WHERE u1.age > 20 
  AND u2.age > 20
  and u1.age > u2.age
  AND oo.amount > 1000 
  AND p.category_id = 5;
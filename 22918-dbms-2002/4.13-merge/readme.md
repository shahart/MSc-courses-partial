# מיון מיזוג חיצוני

## Q

1.	הדרישה

1.1.	יש לממש אלגוריתם מיון מיזוג חיצוני.

1.2.	האלגוריתם יפעל על קובץ טבלאי.

1.3.	התוכנית תמיין את הקובץ לפי רשומה כלשהי.

1.4.	התכנית תעשה שימוש בשכבות אשר מומשו בתרגיל 4.12, כאשר קובץ הקלט וקובץ הפלט יהיו קבצי ערימה.

2.	הנחות ייסוד:

2.1.	התכנית תתייחס לתוכן הרשומה כאל מחרוזת.

2.2.	האלגוריתם פועל על קובץ של מחרוזות. לציון סוף הקובץ הנ"ל משמש הקבוע "DB_EOF". כמו כן יש בתוכנית שימוש בקבוע "NOVAL", שפירושו ערך לא חוקי (או רשומה ריקה – ללא ערך). ההנחה בתוכנית היא כי אין בקובץ הקלט רשומות בעלות ערכים אלה (ההגבלה מתייחסת לרשומה לפיה נדרש למיין).

2.3.	 בנוסף לכך, נשארות בתוקף כל ההנחות וההגבלות כפי שבאו לידי ביטוי במסמך המצורף לתרגיל 4.12.

2.4.	בתוכנית הוגדרו קבועים המגדירים את גודל הקובץ המרבי המותר, את גודל החוצץ, את גודל העמוד (מס' הרשומות) ואת גודלה המרבי של המחרוזת.
יש לוודא כי קובץ הקלט עומד במגבלות אשר מכתיבים קבועים אלה (ניתן, כמובן, לשנות את הקבועים).

2.5.	בקובץ הביניים עליו יופעל האלגוריתם (אשר יכיל רק את הרשומה אותה נדרש למיין), כל העמודים מלבד (אולי) האחרון יהיו מלאים. 

## A

3.	תיאור הפיתרון המוצע:

3.1.	כנדרש, יילקחו מתרגיל 4.13 השכבות הבאות:

3.1.1.	מנהל הדיסק

3.1.2.	מנהל החוצץ

3.1.3.	מנהל קובץ הערימה

3.2.	תמומש שכבת ביניים. תכליתה של שכבת ביניים זו יהיה ליצור קובץ של מחרוזות עליו יופעל אלגוריתם המיון.

תוצאת המיון תיכתב לקובץ זה גם היא.

3.3.	קובץ המקור וקובץ היעד הממוין יכתבו לדיסק תוך שימוש בשכבות המפורטות בסעיף ‎3.1.

4.	תיאור אלגוריתם  מיון מיזוג חיצוני:

4.1.	בצע מעבר pass 0:

4.1.1.	מלא את החוצץ ב – B עמודים, כאשר B הוא גודל החוצץ.

4.1.2.	מיין את – B העמודים.

4.1.3.	כתוב את – B העמודים חזרה לקובץ

4.1.4.	חזור על סעיף ‎4.1.1 עבור B העמודים הבאים, עד למעבר על כל הקובץ.

4.2.	בצע את ה – pass הבא:

4.2.1.	מלא B – 1 עמודים בחוצץ מתוך הקובץ.

4.2.2.	מזג אותם – את התוצאה הכנס לעמוד B בחוצץ.

4.2.2.1.	בכל פעם מזג את עמודים מ – B – 1 ה – run – ים הנוכחיים.

4.2.2.2.	עבור ל – B-1 ה – run – ים הבאים (ה – step הבא).

4.3.	חזור לסעיף ‎4.2, עד אשר יישאר run אחד.

5.	תיאור המודולים

5.1.	מימוש מנהל הדיסק, מנהל החוצץ ומנהל קובץ הערימה יילקחו מהמימוש המקורי.

5.2.	האלגוריתם ימומש במודולים הבאים:

5.2.1.	מודול data_management – ניהול שכבת הביניים – קבצים של מחרוזות – עליה יופעל האלגוריתם.

5.2.2.	מודול extsort_algorithm – מימוש אלגוריתם מיון מיזוג חיצוני

5.2.3.	הגדרות קבועים והגדרות גלובליות אחרות יינתנו ב – global_defs.h.
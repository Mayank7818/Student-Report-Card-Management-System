-- Student Report Card Management System — database schema
--
--   mysql -u root -p < schema.sql

CREATE DATABASE IF NOT EXISTS mydb;
USE mydb;

-- One row per student. RollNo is the primary key, which is what makes the
-- application's INSERT IGNORE seeding idempotent across restarts.
CREATE TABLE IF NOT EXISTS Student (
    RollNo VARCHAR(20)  NOT NULL,
    Name   VARCHAR(100) NOT NULL,
    Avg    DECIMAL(5,2) NOT NULL DEFAULT 0.00,
    Grade  VARCHAR(5)   NOT NULL DEFAULT 'N/A',
    PRIMARY KEY (RollNo)
) ENGINE = InnoDB;

-- Individual subject scores, so a report card can be reproduced and audited
-- instead of only storing the average it was derived from.
CREATE TABLE IF NOT EXISTS mark (
    Id      INT          NOT NULL AUTO_INCREMENT,
    RollNo  VARCHAR(20)  NOT NULL,
    Subject VARCHAR(50)  NOT NULL,
    Score   INT          NOT NULL,
    PRIMARY KEY (Id),
    KEY idx_mark_rollno (RollNo),
    CONSTRAINT fk_mark_student FOREIGN KEY (RollNo)
        REFERENCES Student (RollNo)
        ON UPDATE CASCADE
        ON DELETE CASCADE,
    CONSTRAINT chk_score_range CHECK (Score BETWEEN 0 AND 100)
) ENGINE = InnoDB;

-- Upgrading an older install that had no primary key? Remove the duplicate
-- rows created by the previous version, then add the key:
--
--   DELETE s1 FROM Student s1
--     INNER JOIN Student s2
--     WHERE s1.RollNo = s2.RollNo AND s1.Name > s2.Name;
--   ALTER TABLE Student ADD PRIMARY KEY (RollNo);

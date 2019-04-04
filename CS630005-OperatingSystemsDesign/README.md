# CS 630005 - Operating Systems Design
## Introduction
This project is written by C++ under Qt SDK.

The database used was MySQL. The program was compiled in Qt framework. Qt has an SQL module to connect the database and the application. QSqlDatabase connects the database while the QSqlQuery function sends SQL commands to the SQL server. 

One of the big problem encountered was the amount of setup required just to connect to the database. Despite the large amount of theory we learned in class, practical application still proves to be quite difficult. It is hard to consider and plan for all possible situations and conditions for the application. There was a lot of trial-and-error.

## Project Describtion
TJIN is a payment network, similar to Venmo and Zelle.

User can sign up, sign in, link/unlink bank accounts, verify bank accounts. 

User can send money to another user and request money from other users.

All the data should store in Database.

## SQL table

### bank_account
| NAME             | ATTRIBUTE
| :--------------  |:--------------  
| BankID           | varchar(9), NOT NULL  
| BANumber         | varchar(9), NOT NULL

PRIMARY KEY ('BankID','BANumber')

### electronic_address
| NAME       | ATTRIBUTE
| :-------   | :-------
| Type       | varchar(1),  NOT NULL
| SSN        | varchar(9),  NOT NULL
| Identifier | varchar(45), NOT NULL
| Verified   | varchar(1),  NOT NULL,  DEFAULT 'n'

 PRIMARY KEY (`Identifier`)
 
 UNIQUE KEY `Type_UNIQUE` (`Identifier`)
 
 KEY `SSN_idx` (`SSN`)
 
 CONSTRAINT `SSN` FOREIGN KEY (`SSN`) REFERENCES `user_account` (`ssn`)

### from_
  `RTid` varchar(9) NOT NULL,
  `Identifier` varchar(45) NOT NULL,
  `Percentage` double NOT NULL,
  `Status` varchar(1) NOT NULL DEFAULT 'p',
  KEY `RTid_idx` (`RTid`),
  KEY `Identifier_idx` (`Identifier`),
  CONSTRAINT `Identifier` FOREIGN KEY (`Identifier`) REFERENCES `electronic_address` (`identifier`),
  CONSTRAINT `RTid` FOREIGN KEY (`RTid`) REFERENCES `request_transaction` (`rtid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci

CREATE TABLE `has_additional` (
  `SSN` varchar(9) NOT NULL,
  `BankID` varchar(9) NOT NULL,
  `BANumber` varchar(9) NOT NULL,
  `Verified` varchar(1) NOT NULL DEFAULT 'N',
  PRIMARY KEY (`SSN`,`BankID`,`BANumber`),
  KEY `BANK2_idx` (`BANumber`,`BankID`),
  CONSTRAINT `SSN2` FOREIGN KEY (`SSN`) REFERENCES `user_account` (`ssn`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci

CREATE TABLE `register_account` (
  `USERNAME` varchar(15) NOT NULL,
  `PASSWORD` varchar(15) NOT NULL,
  `SSN` varchar(9) NOT NULL,
  PRIMARY KEY (`USERNAME`),
  UNIQUE KEY `USERNAME_UNIQUE` (`USERNAME`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci

CREATE TABLE `request_transaction` (
  `RTid` varchar(9) NOT NULL,
  `Amount` double NOT NULL,
  `DateTime` datetime NOT NULL,
  `Memo` varchar(45) DEFAULT NULL,
  `SSN` varchar(9) NOT NULL,
  PRIMARY KEY (`RTid`),
  UNIQUE KEY `RTid_UNIQUE` (`RTid`),
  KEY `SSN_idx` (`SSN`),
  CONSTRAINT `SSN1` FOREIGN KEY (`SSN`) REFERENCES `user_account` (`ssn`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
CREATE TABLE `send_to_bank` (
  `SSN` varchar(9) DEFAULT NULL,
  `Amount` double DEFAULT NULL,
  `DateTime` datetime DEFAULT NULL,
  KEY `SSN3_idx` (`SSN`),
  CONSTRAINT `SSN3` FOREIGN KEY (`SSN`) REFERENCES `user_account` (`ssn`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci

CREATE TABLE `send_transaction` (
  `STid` varchar(9) NOT NULL,
  `Amount` double NOT NULL,
  `DateTime` datetime NOT NULL,
  `Memo` varchar(45) DEFAULT NULL,
  `Cancelled` varchar(1) NOT NULL DEFAULT 'n',
  `SSN` varchar(9) NOT NULL,
  `Identifier` varchar(45) NOT NULL,
  PRIMARY KEY (`STid`),
  UNIQUE KEY `STid_UNIQUE` (`STid`),
  KEY `SSN_idx` (`SSN`),
  KEY `Identifier_idx` (`Identifier`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci

CREATE TABLE `user_account` (
  `SSN` varchar(9) NOT NULL,
  `Name` varchar(15) NOT NULL,
  `Balance` double NOT NULL DEFAULT '0',
  `BankID` varchar(9) DEFAULT NULL,
  `BANumber` varchar(9) DEFAULT NULL,
  `PBAVerified` char(1) NOT NULL DEFAULT 'n',
  PRIMARY KEY (`SSN`),
  KEY `BID_idx` (`BankID`),
  KEY `BANK_idx` (`BankID`,`BANumber`),
  CONSTRAINT `BANK` FOREIGN KEY (`BankID`, `BANumber`) REFERENCES `bank_account` (`bankid`, `banumber`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci

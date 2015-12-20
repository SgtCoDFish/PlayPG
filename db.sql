DROP TABLE IF EXISTS `characters`;
DROP TABLE IF EXISTS `players`;

CREATE TABLE `players` (
  `id` int(11) NOT NULL UNIQUE AUTO_INCREMENT,

  `email` varchar(255) NOT NULL UNIQUE,
  `password` varchar(255) NOT NULL,

  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=0;

CREATE TABLE `characters` (
  `id` int(11) NOT NULL UNIQUE AUTO_INCREMENT,

  `playerID` int(11) NOT NULL,

  `name` varchar(12) UNIQUE NOT NULL,

  `baseMaxHP` int(11) NOT NULL,
  `baseStrength` int(11) NOT NULL,
  `baseIntelligence` int(11) NOT NULL,

  PRIMARY KEY (`id`),

  FOREIGN KEY (`playerID`)
    REFERENCES players(`id`)
    ON UPDATE CASCADE
    ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=0;
  

LOCK TABLES `players` WRITE;

INSERT INTO `players` VALUES (1, 'SgtCoDFish@example.com', 'testa');
INSERT INTO `players` VALUES (2, 'ad299@example.com', 'testa');

LOCK TABLES `characters` WRITE;

INSERT INTO `characters` VALUES (1, 1, 'Kurxka', 100, 5, 2);

UNLOCK TABLES;
